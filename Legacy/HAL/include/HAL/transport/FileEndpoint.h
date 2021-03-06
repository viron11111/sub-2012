#ifndef HAL_FILEENDPOINT_H
#define HAL_FILEENDPOINT_H

#include "HAL/transport/BaseEndpoint.h"
#include <boost/thread.hpp>
#include <string>

#include <fcntl.h>
#include <unistd.h>

namespace subjugator {
	class FileEndpoint : public BaseEndpoint {
		public:
			FileEndpoint(const std::string &filename);
			~FileEndpoint(){close();}

			virtual void open();
			virtual void close();
			virtual void write(ByteVec::const_iterator begin, ByteVec::const_iterator end);

		private:
			std::string mFileName;
			int fileDesc;

			boost::thread readthread;
			void readthread_run();

			boost::thread writethread;
			void writethread_run();
			ByteVec writebuf;
			boost::mutex writemutex;
			boost::condition_variable writecond;
	};
}

#endif	// HAL_FILEENDPOINT_H

