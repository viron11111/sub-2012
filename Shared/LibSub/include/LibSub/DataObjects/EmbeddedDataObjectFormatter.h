#ifndef LIBSUB_DATAOBJECTS_EMBEDDEDDATAOBJECTFORMATTER_H
#define LIBSUB_DATAOBJECTS_EMBEDDEDDATAOBJECTFORMATTER_H

#include "LibSub/DataObjects/EmbeddedTypeCodes.h"
#include "HAL/format/DataObjectFormatter.h"
#include <boost/cstdint.hpp>

namespace subjugator {
	class EmbeddedDataObjectFormatter : public DataObjectFormatter {
		public:
			EmbeddedDataObjectFormatter(boost::uint8_t devaddress, boost::uint8_t pcaddress, EmbeddedTypeCode typecode);

			virtual DataObject *toDataObject(const Packet &packet);
			virtual Packet toPacket(const DataObject &dobj);

		protected:
			virtual DataObject *makeInfoDataObject(ByteVec::const_iterator begin, ByteVec::const_iterator end) { return NULL; }

		private:
			boost::uint8_t devaddress;
			boost::uint8_t pcaddress;
			EmbeddedTypeCode typecode;

			boost::uint16_t packetcount_out;
			boost::uint16_t packetcount_in;
	};

	template <typename InfoT>
	class SimpleEmbeddedDataObjectFormatter : public EmbeddedDataObjectFormatter {
		public:
			SimpleEmbeddedDataObjectFormatter(boost::uint8_t devaddress, boost::uint8_t pcaddress, EmbeddedTypeCode typecode)
			: EmbeddedDataObjectFormatter(devaddress, pcaddress, typecode) { }

		protected:
			virtual DataObject *makeInfoDataObject(ByteVec::const_iterator begin, ByteVec::const_iterator end) {
				return new InfoT(begin, end);
			}
	};
}

#endif

