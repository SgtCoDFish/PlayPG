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

#ifndef INCLUDE_NET_PACKETS_LOGINPACKETS_HPP_
#define INCLUDE_NET_PACKETS_LOGINPACKETS_HPP_

#include <string>

#include <APG/s11n/JSON.hpp>

#include "util/Util.hpp"
#include "net/Packet.hpp"
#include "Map.hpp"

#include <openssl/md5.h>

namespace PlayPG {

/**
 * Challenges the client to authenticate themselves to the server.
 *
 * They should check their version against the version of the server, sent in this packet,
 * and use the pubKey to encrypt their details so that only the server can read it.
 */
class AuthenticationChallenge final : public ServerPacket {
public:
	explicit AuthenticationChallenge(const std::string &version, const std::string &versionHash,
	        const std::string &name, const std::string &pubKey);

	const std::string version;
	const std::string versionHash;
	const std::string name;

	const std::string pubKey;
};

class AuthenticationResponse final : public ServerPacket {
public:
	explicit AuthenticationResponse(bool successful_, int attemptsRemaining_, const std::string &message_);

	const bool successful;

	const int attemptsRemaining;

	const std::string message;
};

class ServerPubKey final : public ServerPacket {
public:
	/**
	 * Creates a packet sent to clients of the server with the server's public key.
	 * Clients can use this key to ensure that only the server can read the contents of a
	 * packet they encrypt.
	 * @param pubKeyPEM A public key, stored in PEM format.
	 */
	explicit ServerPubKey(const std::string &pubKeyPEM);

	const std::string pubKey;
};

/**
 * Filled by a client, contains their login details.
 *
 * TODO: Add SSL/encryption
 */
class AuthenticationIdentity final : public ClientPacket {
public:
	explicit AuthenticationIdentity(const std::string &username, std::vector<uint8_t> password);

	uint16_t unameLength;
	std::string username;

	std::vector<uint8_t> password;

	std::string json;
};

class MapServerRegistrationRequest final : public ServerPacket {
public:
	explicit MapServerRegistrationRequest(const std::string &mapServerFriendlyName_);

	const std::string mapServerFriendlyName;
};

class MapServerRegistrationResponse final : public ServerPacket {
public:
	explicit MapServerRegistrationResponse(const std::string &secretRSA_);

	const std::string secretRSA;
};

/**
 * Sent from map server to login server to indicate what maps the map server can support.
 */
class MapServerMapList final : public ServerPacket {
public:
	explicit MapServerMapList(const std::vector<MapIdentifier> &mapHashes);

	const std::vector<MapIdentifier> mapHashes;
};

class VersionMismatch final : public ClientPacket {
public:
	explicit VersionMismatch();
};

}

namespace APG {

template<> class JSONSerializer<PlayPG::AuthenticationChallenge> : public JSONCommon {
public:
	JSONSerializer() :
			        JSONCommon() {

	}

	PlayPG::AuthenticationChallenge fromJSON(const char *json) {
		rapidjson::Document d;

		d.Parse(json);

		return PlayPG::AuthenticationChallenge(d["version"].GetString(), d["versionHash"].GetString(),
		        d["name"].GetString(), d["pubKey"].GetString());
	}

	std::string toJSON(const PlayPG::AuthenticationChallenge &t) {
		buffer.Clear();

		writer->StartObject();

		writer->String("name");
		writer->String(t.name.c_str());

		writer->String("version");
		writer->String(t.version.c_str());

		writer->String("versionHash");
		writer->String(t.versionHash.c_str());

		writer->String("pubKey");
		writer->String(t.pubKey.c_str());

		writer->EndObject();

		return std::string(buffer.GetString());
	}
};

template<> class JSONSerializer<PlayPG::AuthenticationIdentity> : public JSONCommon {
public:
	JSONSerializer() :
			        JSONCommon() {

	}

	PlayPG::AuthenticationIdentity fromJSON(const char *json) {
		rapidjson::Document d;

		d.Parse(json);

		const std::vector<uint8_t> chrs = PlayPG::ByteArrayUtil::hexStringToByteVector(d["password"].GetString());

		return PlayPG::AuthenticationIdentity(d["username"].GetString(), std::move(chrs));
	}

	std::string toJSON(const PlayPG::AuthenticationIdentity &t) {
		buffer.Clear();

		writer->StartObject();

		writer->String("username");
		writer->String(t.username.c_str());

		writer->String("password");

		writer->String(PlayPG::ByteArrayUtil::byteVectorToString(t.password).c_str());

		writer->EndObject();

		return std::string(buffer.GetString());
	}
};

template<> class JSONSerializer<PlayPG::AuthenticationResponse> : public JSONCommon {
public:
	JSONSerializer() :
			        JSONCommon() {

	}

	PlayPG::AuthenticationResponse fromJSON(const char *json) {
		rapidjson::Document d;

		d.Parse(json);

		return PlayPG::AuthenticationResponse(d["successful"].GetBool(), d["attemptsRemaining"].GetInt(),
		        d["message"].GetString());
	}

	std::string toJSON(const PlayPG::AuthenticationResponse &t) {
		buffer.Clear();

		writer->StartObject();

		writer->String("successful");
		writer->Bool(t.successful);

		writer->String("attemptsRemaining");
		writer->Int(t.attemptsRemaining);

		writer->String("message");
		writer->String(t.message.c_str());

		writer->EndObject();

		return std::string(buffer.GetString());
	}
};

template<> class JSONSerializer<PlayPG::MapServerMapList> final : public JSONCommon {
public:
	JSONSerializer() :
			        JSONCommon() {

	}

	PlayPG::MapServerMapList fromJSON(const char *json) {
		rapidjson::Document d;

		d.Parse(json);

		const auto &mapHashes = d["mapHashes"];

		assert(mapHashes.IsArray());

		std::vector<PlayPG::MapIdentifier> ret;

		for (rapidjson::SizeType i = 0; i < mapHashes.Size(); ++i) {
			const auto &object = mapHashes[i];

			const std::string mapName = object["name"].GetString();
			const std::string mapHash = object["hash"].GetString();

			ret.emplace_back(PlayPG::MapIdentifier(mapName, mapHash));
		}

		return PlayPG::MapServerMapList(std::move(ret));
	}

	std::string toJSON(const PlayPG::MapServerMapList &t) {
		buffer.Clear();

		writer->StartObject();

		for (const auto &mapIdentifier : t.mapHashes) {
			writer->StartArray();

			writer->String("name");
			writer->String(mapIdentifier.mapName.c_str());

			writer->String("hash");
			writer->String(mapIdentifier.mapHash.c_str());

			writer->EndArray();
		}

		writer->EndObject();

		return std::string(buffer.GetString());
	}
};

}

#endif /* INCLUDE_NET_PACKETS_LOGINPACKETS_HPP_ */
