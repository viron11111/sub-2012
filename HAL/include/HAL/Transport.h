/**
\class subjugator::Transport
\brief HAL Abstraction for a communication medium (serial, udp, tcp)
*/


#ifndef HAL_TRANSPORT_H
#define HAL_TRANSPORT_H

#include "HAL/shared.h"
#include "HAL/Endpoint.h"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

namespace subjugator {
	class Transport {
		public:
			typedef std::map<std::string, std::string> ParamMap;

			virtual ~Transport() { }

			virtual const std::string &getName() const;
			virtual Endpoint *makeEndpoint(const std::string &address, const ParamMap &params);

	};
}

#endif

