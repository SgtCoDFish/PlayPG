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

#include <utility>

#include <glm/glm.hpp>

#include <Ashley/Ashley.hpp>

#include <tmxparser/Tmx.h>

#include <APG/APG.hpp>

#include "components/Position.hpp"
#include "components/Renderable.hpp"
#include "Map.hpp"

namespace PlayPG {

Map::Map(APG::GLTmxRenderer * const renderer_) :
		        renderer { renderer_ },
		        map { renderer->getMap() } {
	parseMap();
}

const MapTile &Map::getTile(uint32_t x, uint32_t y) const {
	return tiles[resolveTileLoc(x, y)];
}

void Map::parseMap() {
	const auto logger = el::Loggers::getLogger("PlayPG");

	const size_t mapArea = map->GetWidth() * map->GetHeight();
	tiles.reserve(mapArea);

	parseTiles(logger);

	for (const auto &objectGroup : map->GetObjectGroups()) {
		for (const auto &object : objectGroup->GetObjects()) {
			glm::vec2 objectCentre { object->GetX() + object->GetWidth() / 2, object->GetY() + object->GetHeight() / 2 };
			logger->info("Object \"%v\" located at (%v, %v).", object->GetName(), objectCentre.x, objectCentre.y);
		}
	}
}

void Map::parseLayers(el::Logger * const logger) {

}

void Map::parseTiles(el::Logger * const logger) {
	for (int y = 0; y < map->GetHeight(); ++y) {
		for (int x = 0; x < map->GetWidth(); ++x) {
			bool solid = false, interesting = false, spawn = false;

			for (const auto &layer : map->GetTileLayers()) {
				if (layer->GetTileTilesetIndex(x, y) != -1) {
					const auto &properties = layer->GetProperties();
					const auto solidProp = properties.GetStringProperty("solid");

					if (solidProp != "") {
						solid = true;
					}

					const auto tmxTile = map->GetTileset(layer->GetTileTilesetIndex(x, y))->GetTile(
					        layer->GetTileId(x, y));
					if (tmxTile != nullptr) {
						const auto &tilePs = tmxTile->GetProperties().GetStringProperty("interesting");

						if (tilePs != "") {
							logger->verbose(1, "Interesting tile at (x, y) = (%v, %v).", x, y);
							interesting = true;
						}
					}

					if (layer->GetName() == "__playerSpawn") {
						// any laid tile counts as a spawn point on a __playerSpawn level.

						// at the moment, the last detected spawn point will be used as the definitive point.
						logger->verbose(1, "Found spawn point at (x, y) = (%v, %v).", x, y);
						spawnPoint.x = x;
						spawnPoint.y = y;
					}
				}
			}

			tiles.emplace_back(solid, interesting, spawn);
		}
	}
}

std::vector<std::unique_ptr<ashley::Entity>> Map::generateLayerEntities() const {
	std::vector<std::unique_ptr<ashley::Entity>> layerEntities;

	for (int i = 0; i < map->GetNumTileLayers(); ++i) {
		const auto layer = map->GetTileLayer(i);

		if(!layer->IsVisible()) {
			continue;
		}

		layerEntities.emplace_back(std::make_unique<ashley::Entity>());

		layerEntities.back()->add<Position>();
		layerEntities.back()->add<Renderable>(renderer, i);
	}

	return layerEntities;
}

}
