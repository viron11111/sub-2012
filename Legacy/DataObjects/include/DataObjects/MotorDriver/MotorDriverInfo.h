#ifndef DATAOBJECTS_MOTORDRIVER_MOTORDRIVERINFO
#define DATAOBJECTS_MOTORDRIVER_MOTORDRIVERINFO

#include "HAL/format/DataObject.h"
#include <boost/cstdint.hpp>
#include <cmath>

namespace subjugator {
	class MotorDriverInfo : public DataObject {
		public:
			MotorDriverInfo();
			MotorDriverInfo(ByteVec::const_iterator begin, ByteVec::const_iterator end);

			static const int Length = 12;

			int getTickCount() const { return tickcount; }
			double getReferenceInput() const { return refinput; }
			double getPresentOutput() const { return presentoutput; }
			double getRailVoltage() const { return railvoltage; }
			double getCurrent() const { return refinput < 0 ? current : -current; }

			enum ControlType {
				CLOSED,
				OPEN
			};

			enum MotorType {
				BRUSHED,
				BRUSHLESS
			};

			bool getValidMotor() const { return (flags & (1 << 0)) != 0; }
			ControlType getControlType() const { return (flags & (1 << 1)) != 0 ? CLOSED : OPEN; }
			MotorType getMotorType() const { return (flags & (1 << 2)) != 0 ? BRUSHLESS : BRUSHED; }
			bool getHeartbeat() const { return (flags & (1 << 3)) != 0; }
			bool getUnderVoltage() const { return (flags & (1 << 4)) != 0; }
			bool getOverCurrent() const { return (flags & (1 << 5)) != 0; }

		private:
			int tickcount;
			int flags;
			double refinput;
			double presentoutput;
			double railvoltage;
			double current;

			// TODO common header somewhere
			template <int fractional>
			static double convertFixedPoint(uint16_t val) {
				return (double)val / std::pow(2.0, fractional);
			}

			template <int fractional>
			static double convertSignedFixedPoint(uint16_t val) {
				bool negative = (val & 0x8000) != 0;
				double d = (double)(val & ~0x8000) / std::pow(2.0, fractional);
				return negative ? -d : d;
			}
	};
}

#endif
