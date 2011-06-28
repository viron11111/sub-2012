#include "DDSCommanders/PDDDSCommander.h"
#include "SubMain/Workers/PDWorker/SubPDWorker.h"
#include "DataObjects/PD/PDWrench.h"
#include <boost/bind.hpp>

using namespace subjugator;
using namespace boost;

PDDDSCommander::PDDDSCommander(Worker &worker, DDSDomainParticipant *participant)
: wrenchreceiver(participant, "PDWrench", bind(&PDDDSCommander::receivedWrench, this, _1)) {
	screwcmdtoken = worker.ConnectToCommand(PDWorkerCommands::SetScrew, 5);
}

void PDDDSCommander::receivedWrench(const PDWrenchMessage &wrench) {
	PDWrench::Vector6D vec;
	for (int i=0; i<3; i++)
		vec(i) = wrench.linear[i];
	for (int i=0; i<3; i++)
		vec(i+3) = wrench.moment[i];
	screwcmdtoken.lock()->Operate(PDWrench(vec));
}

