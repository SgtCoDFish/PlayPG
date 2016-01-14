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

#include <cassert>

#include <string>
#include <vector>

#include <APG/s11n/JSON.hpp>

#include "util/Util.hpp"
#include "net/Packet.hpp"
#include "Map.hpp"
#include "Character.hpp"

#ifndef INCLUDE_NET_PACKETS_CHARACTERPACKETS_HPP_
#define INCLUDE_NET_PACKETS_CHARACTERPACKETS_HPP_

namespace PlayPG {

class ClientRequestCharacters final : public ClientPacket {
public:
	explicit ClientRequestCharacters() :
			        ClientPacket(ClientOpcode::REQUEST_CHARACTERS) {
	}
};

/**
 * Holds information about every character attached to a given players's account.
 */
class PlayerCharacters final : public ServerPacket {
public:
	explicit PlayerCharacters(const std::vector<Character> &characters_);
	explicit PlayerCharacters(std::vector<Character> &&characters_);

	const std::vector<Character> characters;

private:
	void putJSON();
};

class CharacterSelect final : public ClientPacket {
public:
	explicit CharacterSelect(const Character &character_);
	explicit CharacterSelect(const uint64_t &characterID_);

	const uint64_t characterID;
};

}

namespace APG {

template<> class JSONSerializer<PlayPG::PlayerCharacters> : public JSONCommon {
public:
	JSONSerializer() :
			        JSONCommon() {
	}

	PlayPG::PlayerCharacters fromJSON(const char *json) {
		rapidjson::Document d;

		d.Parse(json);

		const auto &charObject = d["characters"];

		assert(charObject.IsArray());

		std::vector<PlayPG::Character> characters;

		for (rapidjson::SizeType i = 0; i < charObject.Size(); ++i) {
			const auto &playerObject = charObject[i];

			const auto name = playerObject["name"].GetString();

			const auto maxHP = playerObject["maxHP"].GetInt();
			const auto str = playerObject["strength"].GetInt();
			const auto intelligence = playerObject["intelligence"].GetInt();

			characters.emplace_back(name, maxHP, str, intelligence);

			characters.back().id = static_cast<uint64_t>(playerObject["id"].GetInt64());
			characters.back().playerID = static_cast<uint64_t>(playerObject["playerID"].GetInt64());
		}

		return PlayPG::PlayerCharacters(std::move(characters));
	}

	std::string toJSON(const PlayPG::PlayerCharacters &t) {
		buffer.Clear();

		writer->StartObject();
		writer->String("characters");

		writer->StartArray();

		for (const auto &character : t.characters) {
			character.toJson(writer.get());
		}

		writer->EndArray();

		writer->EndObject();

		return std::string(buffer.GetString());
	}
};

}

#endif /* INCLUDE_NET_PACKETS_CHARACTERPACKETS_HPP_ */
