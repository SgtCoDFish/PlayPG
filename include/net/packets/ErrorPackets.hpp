#ifndef INCLUDE_NET_PACKETS_ERRORPACKETS_HPP_
#define INCLUDE_NET_PACKETS_ERRORPACKETS_HPP_

namespace PlayPG {

class MalformedPacket final : public ServerPacket {
public:
	explicit MalformedPacket() :
			        ServerPacket(ServerOpcode::MALFORMED_PACKET) {
	}
};

}

#endif /* INCLUDE_NET_PACKETS_ERRORPACKETS_HPP_ */
