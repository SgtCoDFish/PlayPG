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

#ifndef INCLUDE_PLAYPGMAP_HPP_
#define INCLUDE_PLAYPGMAP_HPP_

#include <cstdint>

#include <vector>

#include <glm/vec2.hpp>

#include <Ashley/Ashley.hpp>
#include <tmxparser/Tmx.h>
#include <APG/core/APGeasylogging.hpp>

namespace PlayPG {

struct MapTile final {
	const bool isSolid;
	const bool isInteresting;
	const bool isSpawn;

	explicit MapTile(bool isSolid = false, bool isInteresting = false, bool isSpawn = false) :
			        isSolid { isSolid },
			        isInteresting { isInteresting },
			        isSpawn { isSpawn } {
	}
};

class Map final {
public:
	static std::string resolveNameFromMap(const Tmx::Map * map, el::Logger * const logger);

	explicit Map(const Tmx::Map * map);

	bool isSolidAtTile(uint32_t x, uint32_t y) const {
		return getTile(x, y).isSolid;
	}

	bool isSolidAtTile(const glm::ivec2 &pos) {
		return getTile(pos.x, pos.y).isSolid;
	}

	bool isSolidAtCoord(float x, float y) const {
		return getTile(x / getTileWidth(), y / getTileHeight()).isSolid;
	}

	bool isSolidAtCoord(const glm::vec2 &pos) {
		return getTile(pos.x / getTileWidth(), pos.y / getTileHeight()).isSolid;
	}

	bool isInterestingAtTile(uint32_t x, uint32_t y) const {
		return getTile(x, y).isInteresting;
	}

	bool isInterestingAtTile(const glm::ivec2 &pos) {
		return getTile(pos.x, pos.y).isInteresting;
	}

	bool isInterestingAtCoord(float x, float y) const {
		return getTile(x / getTileWidth(), y / getTileHeight()).isInteresting;
	}

	bool isInterestingAtCoord(const glm::vec2 &pos) {
		return getTile(pos.x / getTileWidth(), pos.y / getTileHeight()).isInteresting;
	}

	const glm::ivec2 &getSpawnPoint() const {
		return spawnPoint;
	}

	inline int getTileWidth() const {
		return map->GetTileWidth();
	}

	inline int getTileHeight() const {
		return map->GetTileHeight();
	}

	const MapTile &getTile(uint32_t x, uint32_t y) const;

	const Tmx::Map * getTmxMap() const {
		return map;
	}

	const std::string getName() const {
		return name_;
	}

private:
	const Tmx::Map * map;

	std::string name_;

	std::vector<MapTile> tiles;
	glm::ivec2 spawnPoint;

	inline size_t resolveTileLoc(uint32_t x, uint32_t y) const {
		return x + map->GetWidth() * y;
	}

	void parseMap();
	void parseLayers(el::Logger * const logger);
	void parseTiles(el::Logger * const logger);
};

class MapIdentifier {
public:
	static std::string makeMD5Hash(const std::string &base64Hash);

	explicit MapIdentifier(const Map &map);
	explicit MapIdentifier(const std::string &name, const std::string &hash);

	const std::string mapName;
	const std::string mapHash;
};

}

#endif /* INCLUDE_MAP_HPP_ */
