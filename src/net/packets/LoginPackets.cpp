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

#include <cstdint>

#include "net/packets/LoginPackets.hpp"

#include "PlayPGVersion.hpp"

namespace PlayPG {

AuthenticationChallenge::AuthenticationChallenge(const std::string &version_, const std::string &versionHash_,
        const std::string &name_, const std::string &pubKey_) :
		        ServerPacket(ServerOpcode::LOGIN_AUTHENTICATION_CHALLENGE),
		        version { version_ },
		        versionHash { versionHash_ },
		        name { name_ },
		        pubKey { pubKey_ } {
	APG::JSONSerializer<AuthenticationChallenge> toJson;

	const std::string json = toJson.toJSON(*this);

	buffer.putShort(static_cast<uint16_t>(json.size()));
	buffer.putString(json);
}

AuthenticationResponse::AuthenticationResponse(bool successful_, int attemptsRemaining_, const std::string &message_) :
		        ServerPacket(ServerOpcode::LOGIN_AUTHENTICATION_RESPONSE),
		        successful { successful_ },
		        attemptsRemaining { attemptsRemaining_ },
		        message { message_ } {
	APG::JSONSerializer<AuthenticationResponse> toJson;

	const std::string json = toJson.toJSON(*this);
	buffer.putShort(static_cast<uint16_t>(json.size()));
	buffer.putString(json);
}

AuthenticationIdentity::AuthenticationIdentity(const std::string &username_, std::vector<uint8_t> password_) :
		        ClientPacket(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY),
		        unameLength { static_cast<decltype(unameLength)>(username_.length()) },
		        username { username_ },
		        password { std::move(password_) } {
	APG::JSONSerializer<AuthenticationIdentity> toJson;

	json = toJson.toJSON(*this);

	buffer.putShort(static_cast<uint16_t>(json.size()));

	buffer.putString(json);
}

ServerPubKey::ServerPubKey(const std::string &pubKeyPEM) :
		        ServerPacket(ServerOpcode::SERVER_PUBKEY),
		        pubKey { pubKeyPEM } {
	buffer.putLong(pubKey.size());

	buffer.putString(pubKey);
}

VersionMismatch::VersionMismatch() :
		        ClientPacket(ClientOpcode::VERSION_MISMATCH) {
}

MapServerRegistrationRequest::MapServerRegistrationRequest(const std::string &mapServerFriendlyName_,
        const std::string &mapServerListenAddress_, const uint16_t &port_) :
		        ServerPacket(ServerOpcode::MAP_SERVER_REGISTRATION_REQUEST),
		        mapServerFriendlyName { mapServerFriendlyName_ },
		        mapServerListenAddress { mapServerListenAddress_ },
		        port { port_ } {
	buffer.putShort(mapServerFriendlyName.size());
	buffer.putString(mapServerFriendlyName);

	buffer.putShort(mapServerListenAddress.size());
	buffer.putString(mapServerListenAddress);

	buffer.putShort(port);
}

MapServerRegistrationResponse::MapServerRegistrationResponse(const std::string &secretRSA_) :
		        ServerPacket(ServerOpcode::MAP_SERVER_REGISTRATION_RESPONSE),
		        secretRSA { secretRSA_ } {
	buffer.putShort(secretRSA.size());
	buffer.putString(secretRSA);
}

MapServerMapList::MapServerMapList(const std::vector<Location> &mapHashes_) :
		        ServerPacket(ServerOpcode::MAP_SERVER_MAP_LIST),
		        mapHashes { mapHashes_ } {
	APG::JSONSerializer<MapServerMapList> toJson;
	const std::string jsonString = toJson.toJSON(*this);

	buffer.putShort(jsonString.size());
	buffer.putString(jsonString);
}

MapServerMapList MapServerMapList::listFromMaps(const std::vector<Map> &maps) {
	std::vector<Location> ids;

	for (const auto &map : maps) {
		ids.emplace_back(Location(map));
	}

	return MapServerMapList(std::move(ids));
}

MapServerConnectionInstructions::MapServerConnectionInstructions(const std::string &friendlyName_,
        const std::string &hostName_, const uint16_t &port_) :
		        ServerPacket(ServerOpcode::MAP_SERVER_CONNECTION_INSTRUCTIONS),
		        friendlyName { friendlyName_ },
		        hostName { hostName_ },
		        port { port_ } {
	APG::JSONSerializer<MapServerConnectionInstructions> jsonS11N;

	const std::string json = jsonS11N.toJSON(*this);

	buffer.putShort(json.size());
	buffer.putString(json);
}

}
