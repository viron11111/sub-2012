#ifndef PRIMITIVEDRIVER_THRUSTER_H
#define PRIMITIVEDRIVER_THRUSTER_H

#include "HAL/HAL.h"
#include "DataObjects/MotorDriver/MotorDriverInfo.h"
#include "LibSub/State/StateUpdater.h"
#include "LibSub/Worker/WorkerEndpoint.h"

namespace subjugator {
	class Thruster : public StateUpdater {
		public:
			Thruster(HAL &hal, int address, int srcaddress);

			boost::shared_ptr<MotorDriverInfo> getInfo() const;
			void setEffort(double effort);

			virtual void updateState(double dt) { endpoint.updateState(dt); }
			virtual const State &getState() const { return endpoint.getState(); }

		private:
			int address;
			WorkerEndpoint endpoint;

			void endpointInitCallback();
	};
}

#endif

