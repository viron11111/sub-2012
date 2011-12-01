#include "PrimitiveDriver/Thruster.h"
#include "HAL/format/Sub7EPacketFormatter.h"
#include "PrimitiveDriver/DataObjects/HeartBeat.h"
#include "LibSub/DataObjects/StartPublishing.h"
#include "PrimitiveDriver/DataObjects/MotorDriverDataObjectFormatter.h"
#include "PrimitiveDriver/DataObjects/SetReference.h"
#include <Eigen/Dense>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

using namespace subjugator;
using namespace Eigen;
using namespace boost;
using namespace std;

Thruster::Thruster(HAL &hal, int address, int srcaddress) :
address(address),
endpoint(WorkerEndpoint::Args()
	.setName("thruster" + lexical_cast<string>(address))
	.setEndpoint(hal.openDataObjectEndpoint(address, new MotorDriverDataObjectFormatter(address, srcaddress, BRUSHEDOPEN), new Sub7EPacketFormatter()))
	.setInitCallback(bind(&Thruster::endpointInitCallback, this))
	.setMaxAge(.2)
) { }

shared_ptr<MotorDriverInfo> Thruster::getInfo() const {
	return endpoint.getDataObject<MotorDriverInfo>();
}

void Thruster::setEffort(double effort) {
	endpoint.write(SetReference(effort));
}

void Thruster::endpointInitCallback() {
	endpoint.write(HeartBeat());
	endpoint.write(StartPublishing(50));
}

void Thruster::updateState(double dt) {
	shared_ptr<MotorDriverInfo> info = getInfo();

	endpoint.updateState(dt);
	if (endpoint.getState().code != State::ACTIVE) {
		state = endpoint.getState();
		return;
	}

	vector<string> errstrs;
	if (info->getOverCurrent())
		errstrs.push_back("over current");
	if (info->getUnderVoltage())
		errstrs.push_back("under voltage");
	if (!info->getValidMotor())
		errstrs.push_back("invalid motor");

	if (errstrs.size() == 0)
		state = State::ACTIVE;
	else
		state = State(State::ERROR, "Thruster " + lexical_cast<string>(address) + " offline because: " + join(errstrs, ", "));
}



