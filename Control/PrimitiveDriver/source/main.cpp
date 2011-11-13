#include "PrimitiveDriver/PDWorker.h"
#include "PrimitiveDriver/Messages/PDWrenchMessageSupport.h"
#include "PrimitiveDriver/Messages/PDActuatorMessageSupport.h"
#include "PrimitiveDriver/Messages/PDStatusMessageSupport.h"
#include "HAL/SubHAL.h"
#include "LibSub/Worker/WorkerRunner.h"
#include "LibSub/Worker/SignalHandler.h"
#include "LibSub/Worker/WorkerConfigLoader.h"
#include "LibSub/DDS/DDSBuilder.h"
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <algorithm>
#include <vector>

using namespace subjugator;
using namespace boost;
namespace po = boost::program_options;
using namespace std;

DECLARE_MESSAGE_TRAITS(PDWrenchMessage);
DECLARE_MESSAGE_TRAITS(PDStatusMessage);
DECLARE_MESSAGE_TRAITS(PDActuatorMessage);

int main(int argc, char **argv) {
	// parse options
	po::options_description desc("PrimitiveDriver options");
	desc.add_options()
		("help", "produce help message")
		("overlays,o", po::value<vector<string> >(), "use configuration overlays")
		("debug,d", "output debug log messages");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cerr << desc;
		return 1;
	}

	asio::io_service io;

	// Get the worker up
	SubHAL hal(io);
	WorkerConfigLoader configloader;
	if (vm.count("overlays")) {
		const vector<string> &overlays = vm["overlays"].as<vector<string> >();
		for_each(overlays.begin(), overlays.end(), boost::bind(&WorkerConfigLoader::addOverlay, &configloader, _1));
	}
	PDWorker worker(hal, configloader);
	OStreamLog(worker.logger, cout, vm.count("debug") ? WorkerLogEntry::DEBUG : WorkerLogEntry::INFO);
	WorkerRunner workerrunner(worker, io);
	SignalHandler sighandler(io);

	// Get DDS up
	DDSBuilder dds(io);
	dds.worker(worker);
	dds.receiver(worker.wrenchmailbox, dds.topic<PDWrenchMessage>("PDWrench", TopicQOS::LEGACY));
	dds.receiver(worker.actuatormailbox, dds.topic<PDActuatorMessage>("PDActuator", TopicQOS::LEGACY));
	dds.sender(worker.infosignal, dds.topic<PDStatusMessage>("PDStatus", TopicQOS::LEGACY));

	// Start the worker
	workerrunner.start();
	io.run();
}

namespace subjugator {
	template <>
	void from_dds(Vector6d &vec, const PDWrenchMessage &msg) {
		for (int i=0; i<3; i++)
			vec(i) = msg.linear[i];
		for (int i=0; i<3; i++)
			vec(i+3) = msg.moment[i];
	}

	template <>
	void from_dds(int &flags, const PDActuatorMessage &actuator) {
		flags = actuator.flags;
	}

	template <>
	void to_dds(PDStatusMessage &msg, const PDInfo &info) {
		msg.timestamp = info.getTimestamp();
		for (int i=0; i<8; i++)
			msg.current[i] = info.getCurrent(i);
		const MergeInfo &mergeinfo = info.getMergeInfo();
		msg.estop = mergeinfo.getESTOP();
		msg.flags = mergeinfo.getFlags();
		msg.tickcount = mergeinfo.getTickCount();
		msg.voltage16 = mergeinfo.getRail16Voltage();
		msg.current16 = mergeinfo.getRail16Current();
		msg.voltage32 = mergeinfo.getRail32Voltage();
		msg.current32 = mergeinfo.getRail32Current();
	}
}

