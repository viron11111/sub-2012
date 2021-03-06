#ifndef TRACKINGCONTROLLER_TRACKINGCONTROLLER_H
#define TRACKINGCONTROLLER_TRACKINGCONTROLLER_H

#include "LibSub/Math/EigenUtils.h"
#include <istream>

namespace subjugator {
	class TrackingController {
		public:
			struct Gains {
				Vector6d k;
				Vector6d ks;
				Vector6d alpha;
				Vector6d beta;
				Vector6d gamma1;
				Vector19d gamma2;
			};

			enum ControlTerms {
				TERM_PD = (1 << 0),
				TERM_RISE = (1 << 1),
				TERM_NN = (1 << 2)
			};

			enum Mode {
				MODE_PD = TERM_PD,
				MODE_RISE = TERM_RISE,
				MODE_RISE_NN = TERM_RISE | TERM_NN
			};

			struct Config {
				Gains gains;
				int mode;
			};

			TrackingController(const Config &config);

			struct State {
				Vector6d x;
				Vector6d vb;
			};

			struct TrajectoryPoint {
				Vector6d xd;
				Vector6d xd_dot;
			};

			struct Output {
				Vector6d control;

				Vector6d control_pd;
				Vector6d control_rise;
				Vector6d control_nn;
			};

			Output update(double dt, const TrajectoryPoint &t, const State &s);

			const Matrix19x5d &getVHat() const { return V_hat_prev; }
			const Matrix6d &getWHat() const { return W_hat_prev; }

		private:
			const Config config;

			Vector6d rise_term_prev;
			Vector6d rise_term_int_prev;

			Vector6d xd_dot_prev;
			Vector6d xd_dotdot_prev;

			// The number of columns defines the number of hidden layer neurons in the controller, for now this is hardcoded at 5
			// Size = 19xN2
			Matrix19x5d V_hat_dot_prev;
			Matrix19x5d V_hat_prev;
			// Size = (N2+1)x6
			Matrix6d W_hat_dot_prev;
			Matrix6d W_hat_prev;

			Vector6d riseFeedbackNoAccel(double dt, const Vector6d &e2);
			Vector6d nnFeedForward(double dt, const Vector6d &e2, const TrajectoryPoint &t);
			Vector6d pdFeedback(double dt, const Vector6d &e2);

			static Matrix6d jacobian(const Vector6d &x);
			static Matrix6d jacobianInverse(const Vector6d &x);
	};

	std::istream &operator>>(std::istream &in, TrackingController::Mode &mode);
}

#endif

