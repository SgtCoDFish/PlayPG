/*
 * Copyright (c) 2015 See AUTHORS file.
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDE_NET_OPCODES_HPP_
#define INCLUDE_NET_OPCODES_HPP_

#include <cstdint>

#include <type_traits>

namespace PlayPG {

struct OpcodeDetails {
	OpcodeDetails(const char *humanReadable_, bool encrypted_ = false) :
			        humanReadable { humanReadable_ },
			        encrypted { encrypted_ } {

	}

	const char * const humanReadable;
	const bool encrypted;
};

enum class ClientOpcode
	: uint16_t {
		LOGIN_AUTHENTICATION_IDENTITY = 0x0001,
	VERSION_MISMATCH = 0x0002,
	REQUEST_CHARACTERS = 0x0003,
	CHARACTER_SELECT = 0x0004,
	MOVE = 0x000A,
};

enum class ServerOpcode
	: uint16_t {
		LOGIN_AUTHENTICATION_CHALLENGE = 0xFFFE,
	LOGIN_AUTHENTICATION_RESPONSE = 0xFFFD,
	MAP_SERVER_REGISTRATION_REQUEST = 0xFFFC,
	MAP_SERVER_REGISTRATION_RESPONSE = 0xFFFB,
	MAP_SERVER_MAP_LIST = 0xFFFA,
	MAP_SERVER_NOT_NEEDED = 0XFFF9,
	MAP_SERVER_ACK = 0xFFF8,
	MAP_SERVER_CONNECTION_INSTRUCTIONS = 0xFFF7,
	PLAYER_CHARACTERS = 0xFFF6,
	NO_MAP_SERVER_ERROR = 0xFFF5,
	SERVER_PUBKEY = 0xFFF4,
	MALFORMED_PACKET = 0xFFF3,
};

static_assert(std::is_same<std::underlying_type<ClientOpcode>::type, std::underlying_type<ServerOpcode>::type>::value, "ClientOpcodes and ServerOpcodes must have the same underlying type.");
using opcode_type_t = std::underlying_type<ClientOpcode>::type;
// can safely use the type of ClientOpcode since we mandate that the underlying type must match that of the server opcodes.
}

#endif /* INCLUDE_NET_OPCODES_HPP_ */
