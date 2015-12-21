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

#include "net/Packet.hpp"

namespace PlayPG {

class AuthenticationChallenge final : public ServerPacket {
public:
	explicit AuthenticationChallenge(const std::string &version, const std::string &versionHash,
	        const std::string &name);

	std::string version;
	std::string versionHash;
	std::string name;
};

class AuthenticationResponse final : public ServerPacket {
public:
	explicit AuthenticationResponse(bool successful_);

	bool successful;
};

/**
 * Filled by a client, contains their login details.
 *
 * TODO: Add SSL/encryption
 */
class AuthenticationIdentity final : public ClientPacket {
public:
	explicit AuthenticationIdentity(const std::string &username, const std::string &password);

	uint16_t unameLength;
	std::string username;

	uint16_t passwordLength;
	std::string password;

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
		        d["name"].GetString());
	}

	std::string toJSON(const PlayPG::AuthenticationChallenge &t) {
		buffer.Clear();

		writer->StartObject();

		writer->String("version");
		writer->String(t.version.c_str());

		writer->String("versionHash");
		writer->String(t.versionHash.c_str());

		writer->String("name");
		writer->String(t.name.c_str());

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

		return PlayPG::AuthenticationIdentity(d["username"].GetString(), d["password"].GetString());
	}

	std::string toJSON(const PlayPG::AuthenticationIdentity &t) {
		buffer.Clear();

		writer->StartObject();

		writer->String("username");
		writer->String(t.username.c_str());

		writer->String("password");
		writer->String(t.password.c_str());

		writer->EndObject();

		return std::string(buffer.GetString());
	}
};

}

#endif /* INCLUDE_NET_PACKETS_LOGINPACKETS_HPP_ */
