#ifndef SubIMUWorker_H
#define SubIMUWorker_H

#include "SubMain/SubPrerequisites.h"
#include "SubMain/Workers/SubWorker.h"
#include "HAL/HAL.h"
#include "HAL/SubHAL.h"
#include "HAL/format/SPIPacketFormatter.h"
#include "DataObjects/IMU/IMUDataObjectFormatter.h"

namespace subjugator
{
	class IMUWorker : public Worker
	{
		public:
			IMUWorker(boost::asio::io_service& io, int64_t rate);
			~IMUWorker()
			{
				if(pEndpoint)
					delete pEndpoint;
			}

			bool Startup();

		private:
			SubHAL hal;
			DataObjectEndpoint* pEndpoint;

			void readyState();
			void emergencyState();
			void failState();
			void halReceiveCallback(std::auto_ptr<DataObject> &dobj);
			void halStateChangeCallback();
	};
}


#endif // SubIMUWorker_H

