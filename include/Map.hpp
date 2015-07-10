/*
 * Copyright (c) 2015 Ashley Davis (SgtCoDFish)
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

#ifndef INCLUDE_MAP_HPP_
#define INCLUDE_MAP_HPP_

#include <cstdint>

#include <vector>

#include <glm/vec2.hpp>

namespace Tmx {
class Map;
}

namespace PlayPG {

struct MapTile {
	const bool isSolid;
	const bool isInteresting;
	const bool isSpawn;

	explicit MapTile(bool isSolid = false, bool isInteresting = false, bool isSpawn = false) :
			isSolid { isSolid }, isInteresting { isInteresting }, isSpawn { isSpawn } {
	}
};

class Map {
private:
	Tmx::Map * map;

	std::vector<MapTile> tiles;
	glm::ivec2 spawnPoint;

	inline size_t resolveTileLoc(uint32_t x, uint32_t y) const {
		return x + map->GetWidth() * y;
	}

	void parseMap();

public:
	explicit Map(const std::unique_ptr<Tmx::Map> &map) :
			Map(map.get()) {
	}
	explicit Map(Tmx::Map * const map);

	bool isSolid(uint32_t x, uint32_t y) const {
		return getTile(x, y).isSolid;
	}

	bool isInteresting(uint32_t x, uint32_t y) const {
		return getTile(x, y).isInteresting;
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
};

}

#endif /* INCLUDE_MAP_HPP_ */
