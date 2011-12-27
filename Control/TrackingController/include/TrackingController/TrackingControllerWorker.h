#ifndef _TRACKINGCONTROLLERWORKER_H__
#define _TRACKINGCONTROLLERWORKER_H__

#include "HAL/HAL.h"
#include "LibSub/Math/EigenUtils.h"
#include "LibSub/State/State.h"
#include "LibSub/Worker/Worker.h"
#include "LibSub/Worker/WorkerConfigLoader.h"
#include "LibSub/Worker/WorkerMailbox.h"
#include "LibSub/Worker/WorkerSignal.h"
#include "LibSub/Worker/WorkerKill.h"
#include "TrackingController/TrackingController.h"
#include <boost/scoped_ptr.hpp>

namespace subjugator {
	class TrackingControllerWorker : public Worker {
		public:
			struct LPOSVSSInfo {
				Vector3d position_ned;
				Vector4d quaternion_ned_b;
				Vector3d velocity_ned;
				Vector3d angularrate_body;
			};

			struct LogData {
				Vector6d x;
				Vector6d x_dot;
				Vector6d xd;
				Vector6d xd_dot;

				Matrix19x5d v_hat;
				Matrix6d w_hat;

				TrackingController::Output out;
			};

			TrackingControllerWorker(const WorkerConfigLoader &configloader);

			WorkerMailbox<LPOSVSSInfo> lposvssmailbox;
			WorkerMailbox<TrackingController::TrajectoryPoint> trajectorymailbox;
			WorkerMailbox<TrackingController::Gains> gainsmailbox;
			WorkerKillMonitor killmon;

			WorkerSignal<Vector6d> wrenchsignal;
			WorkerSignal<LogData> logsignal;

		protected:
			virtual void enterActive();
			virtual void work(double dt);

		private:
			const WorkerConfigLoader &configloader;

			boost::scoped_ptr<TrackingController> controllerptr;
			TrackingController::Config controllerconfig;

			void setControllerGains(const boost::optional<TrackingController::Gains> &gains);

			void loadConfig();
			void resetController();
			void setCurrentPosWaypoint();
	};
}

#endif
