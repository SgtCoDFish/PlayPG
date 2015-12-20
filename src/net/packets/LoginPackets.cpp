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

#include "net/packets/LoginPackets.hpp"

#include "PlayPGVersion.hpp"

namespace PlayPG {

AuthenticationChallenge::AuthenticationChallenge() :
		        ServerPacket(ServerOpcode::LOGIN_AUTHENTICATION_CHALLENGE) {
	buffer.putShort(static_cast<opcode_type_t>(opcode));

	const uint16_t verLen = uint16_t(std::strlen(Version::versionString));
	buffer.putShort(verLen);

	for (int i = 0; i < verLen; i++) {
		buffer.putChar(Version::versionString[i]);
	}

	const uint16_t hashLen = uint16_t(std::strlen(Version::gitHash));
	buffer.putShort(hashLen);

	for (int i = 0; i < hashLen; i++) {
		buffer.putChar(Version::gitHash[i]);
	}
}

AuthenticationIdentity::AuthenticationIdentity(const std::string &username_, const std::string &password_) :
		        ClientPacket(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY),
		        unameLength { static_cast<decltype(unameLength)>(username_.length()) },
		        username { username_ },
		        passwordLength { static_cast<decltype(passwordLength)>(password_.length()) },
		        password { password_ } {
	buffer.putShort(static_cast<opcode_type_t>(opcode));

	buffer.putShort(unameLength);
	for (auto &c : username_) {
		buffer.putChar(c);
	}

	buffer.putShort(passwordLength);
	for (auto &c : password_) {
		buffer.putChar(c);
	}
}

AuthenticationResponse::AuthenticationResponse(bool successful_) :
		        ServerPacket(ServerOpcode::LOGIN_AUTHENTICATION_RESPONSE),
		        successful { successful_ } {
	buffer.putShort(static_cast<opcode_type_t>(opcode));

	buffer.put(static_cast<uint8_t>(successful));
}

}

