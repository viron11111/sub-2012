#include "Hydrophone/DataObjects/HydrophonePacketFormatter.h"
#include <algorithm>

using namespace subjugator;
using namespace std;

HydrophonePacketFormatter::HydrophonePacketFormatter() { }

#define PACKETSIZE 2048

vector<Packet> HydrophonePacketFormatter::parsePackets(ByteVec::const_iterator begin, ByteVec::const_iterator end) {
	buf.insert(buf.end(), begin, end);

	vector<Packet> packets;
	ByteVec::iterator pos = buf.begin();
	while (true) {
		static const uint8_t receive_header[] = { 0xFE, 0xFE };
		pos = search(pos, buf.end(), receive_header, receive_header + sizeof(receive_header)); // consume until just before the receive header
		if (pos == buf.end()) // consumed everything?
			break; // no more packets

		ByteVec::iterator datastart = pos + sizeof(receive_header); // data starts after the header
		if (buf.end() - datastart < PACKETSIZE) // if we don't have enough bytes for a complete packet
			break; // no more packets

		ByteVec::iterator dataend = datastart + PACKETSIZE; // compute the end of the data
		packets.push_back(Packet(datastart, dataend)); // save the packet
		pos = dataend; // consume the entire contents of the packet
	}

	buf.erase(buf.begin(), pos); // erase all consumed data from the buffer
	return packets;
}

ByteVec HydrophonePacketFormatter::formatPacket(const Packet &packet) const {
	return packet; // no real format to outgoing packets
}

