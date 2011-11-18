#ifndef PRIMITIVEDRIVER_PDWORKER_H
#define PRIMITIVEDRIVER_PDWORKER_H

#include "PrimitiveDriver/ThrusterManager.h"
#include "PrimitiveDriver/ThrusterMapper.h"
#include "PrimitiveDriver/MergeManager.h"
#include "LibSub/Worker/Worker.h"
#include "LibSub/Worker/WorkerMailbox.h"
#include "LibSub/Worker/WorkerSignal.h"
#include "LibSub/Worker/WorkerEndpoint.h"
#include "LibSub/Worker/WorkerConfigLoader.h"
#include "LibSub/Math/EigenUtils.h"
#include "HAL/HAL.h"
#include "PrimitiveDriver/DataObjects/PDInfo.h"
#include <Eigen/Dense>
#include <map>

namespace subjugator {
	class PDWorker : public Worker {
		public:
			PDWorker(HAL &hal, const WorkerConfigLoader &configloader);

			WorkerMailbox<Vector6d> wrenchmailbox;
			WorkerMailbox<int> actuatormailbox;

			WorkerSignal<std::vector<double> > currentsignal;
			WorkerSignal<PDInfo> infosignal;

		protected:
			virtual void initialize();
			virtual void work(double dt);

		private:
			void wrenchSet(const boost::optional<Vector6d> &wrench);
			void actuatorSet(const boost::optional<int> &actuators);
			void thrusterStateChanged(int num, const State &state);

			virtual void leaveActive();

			HAL &hal;
			const WorkerConfigLoader &configloader;

			WorkerEndpoint heartbeatendpoint;
			ThrusterManager thrustermanager;
			ThrusterMapper thrustermapper;
			MergeManager mergemanager;

			std::vector<ThrusterMapper::Entry> thrusterentries;
	};
}

#endif // _SubPDWorker_H__

