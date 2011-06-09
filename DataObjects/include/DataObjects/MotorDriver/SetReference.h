#ifndef DATAOBJECTS_MOTORDRIVER_SETREFERENCE_H
#define DATAOBJECTS_MOTORDRIVER_SETREFERENCE_H

#include "DataObjects/MotorDriver/MotorDriverCommand.h"
#include <boost/cstdint.hpp>

namespace subjugator {
	class SetReference : public MotorDriverCommand {
		public:
			SetReference(double reference);

			virtual uint8_t getToken() const { return 3; }
			virtual void appendDataPacket(Packet &packet) const;

		private:
			double reference;
	};
}

#endif
