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

#include "data/Character.hpp"
#include "util/Util.hpp"
#include "net/Packet.hpp"
#include "Map.hpp"

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
			const auto name = charObject[i]["name"].GetString();

			const auto &statsObject = charObject[i]["stats"];

			const auto maxHP = statsObject["maxHP"].GetInt();
			const auto str = statsObject["strength"].GetInt();
			const auto intelligence = statsObject["intelligence"].GetInt();

			PlayPG::Stats stats { maxHP, str, intelligence };

			characters.emplace_back(name, std::move(stats));
		}

		return PlayPG::PlayerCharacters(std::move(characters));
	}

	std::string toJSON(const PlayPG::PlayerCharacters &t) {
		buffer.Clear();

		writer->StartObject();
		writer->String("characters");

		writer->StartArray();

		for (const auto &character : t.characters) {
			writer->StartObject();

			writer->String("name");
			writer->String(character.name.c_str());

			writer->String("stats");
			writer->StartObject();

			writer->String("maxHP");
			writer->Int(character.stats.maxHP);

			writer->String("strength");
			writer->Int(character.stats.strength);

			writer->String("intelligence");
			writer->Int(character.stats.intelligence);

			writer->EndObject();

			writer->EndObject();
		}

		writer->EndArray();

		writer->EndObject();
		return std::string(buffer.GetString());
	}
};

}

#endif /* INCLUDE_NET_PACKETS_CHARACTERPACKETS_HPP_ */
