#ifndef IMU_INFO_H
#define IMU_INFO_H

#include <boost/cstdint.hpp>
#include "HAL/format/DataObject.h"
#include <Eigen/Dense>
#include <math.h>

/*
 * This is not a good example of a dataobject converter. Since the IMU is sitting on the SPI bus,
 * this is guaranteed to be called on the same architecture(endianess) machine as the driver assembling
 * the packet. Hence, the ugly casting does work.
 */
namespace subjugator
{
	class IMUInfo : public DataObject
	{
		public:
			IMUInfo(){}
			IMUInfo(boost::uint16_t flags, double supply, double temp, boost::uint64_t timestamp, Eigen::Vector3d acc,
					Eigen::Vector3d ang, Eigen::Vector3d mag_field):
					flags(flags), supplyVoltage(supply), temperature(temp), timestamp(timestamp), acceleration(acc),
					ang_rate(ang), mag_field(mag_field){}

			static IMUInfo *parse(ByteVec::const_iterator begin, ByteVec::const_iterator end);

			boost::uint64_t getTimestamp()const { return timestamp; }
			double getTemperature() const { return temperature; }
			double getSupplyVoltage() const { return supplyVoltage; }
			double getAngularRateI(int i) const { return ang_rate(i); }
			double getAccelerationI(int i) const { return acceleration(i); }
			double getMagneticFieldI(int i) const { return mag_field(i); }
			const Eigen::Vector3d &getAcceleration() const{ return acceleration; }
			const Eigen::Vector3d &getAngularRate() const { return ang_rate; }
			const Eigen::Vector3d &getMagneticField()const { return mag_field; }
			boost::uint16_t getFlags()	const { return flags;	}

		private:
			// These conversions change the bit values in the registers to useful units
			static const double SUPPLY_CONVERSION = 0.00242; // V

			static const double TEMP_CONVERSION = 0.14;		// degC / bit
			static const double TEMP_CENTER = 25.0;			// Temps are centered around 25.0 degC

			static const double GYRO_CONVERSION = 0.000872664626;	// rad/s
			static const double ACC_CONVERSION = 0.0033;				// g's
			static const double MAG_CONVERSION = 0.0005;			// gauss

			static const int IMU_PACKET_LENGTH = 32;

			static boost::uint16_t getU16LE(ByteVec::const_iterator pos);
			static boost::int16_t getS16LE(ByteVec::const_iterator pos);
			static boost::uint64_t getU64LE(ByteVec::const_iterator pos);

			boost::uint16_t flags;
			double supplyVoltage;
			double temperature;
			boost::uint64_t timestamp;
			Eigen::Vector3d acceleration;
			Eigen::Vector3d ang_rate;
			Eigen::Vector3d mag_field;
	};
}

#endif	// IMU_INFO_H

