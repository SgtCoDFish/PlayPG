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

#include <APG/APG.hpp>

#include "ClientMapUtil.hpp"
#include "Components.hpp"

namespace PlayPG {

std::vector<std::unique_ptr<ashley::Entity>> MapUtil::generateFrontLayerEntities(const Map &ppgMap, APG::GLTmxRenderer *renderer) {
	std::vector<std::unique_ptr<ashley::Entity>> layerEntities;

	const auto map = ppgMap.getTmxMap();

	for (int i = 0; i < map->GetNumTileLayers(); ++i) {
		const auto layer = map->GetTileLayer(i);

		if (!layer->IsVisible()) {
			continue;
		} else if (layer->GetName() == "__playerSpawn") {
			break;
		}

		layerEntities.emplace_back(std::make_unique<ashley::Entity>());

		layerEntities.back()->add<Position>();
		layerEntities.back()->add<Renderable>(renderer, i);
	}

	return layerEntities;
}

std::vector<std::unique_ptr<ashley::Entity>> MapUtil::generateBackLayerEntities(const Map &ppgMap, APG::GLTmxRenderer *renderer) {
	std::vector<std::unique_ptr<ashley::Entity>> layerEntities;
	bool spawnFound = false;

	const auto map = ppgMap.getTmxMap();

	for (int i = 0; i < map->GetNumTileLayers(); ++i) {
		const auto layer = map->GetTileLayer(i);

		if (layer->GetName() == "__playerSpawn") {
			spawnFound = true;
		}

		if (spawnFound) {
			if (!layer->IsVisible()) {
				continue;
			}

			layerEntities.emplace_back(std::make_unique<ashley::Entity>());

			layerEntities.back()->add<Position>();
			layerEntities.back()->add<Renderable>(renderer, i);
		}
	}

	return layerEntities;
}

}
