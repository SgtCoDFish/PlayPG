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

#include <utility>

#include <APG/APGeasylogging.hpp>

#include "PlayPG.hpp"
#include "Map.hpp"

using namespace APG;

el::Logger *PlayPG::PlayPG::logger = nullptr;

PlayPG::PlayPG::PlayPG() :
		APG::SDLGame("PlayPG", 1280u, 720u) {
	PlayPG::logger = el::Loggers::getLogger("PlayPG");
}

bool PlayPG::PlayPG::init() {
	tmxMap = std::make_unique<Tmx::Map>();
	tmxMap->ParseFile("assets/room1.tmx");

	if (tmxMap->HasError()) {
		logger->fatal("Couldn't load room1.tmx");
		return false;
	}

	map = std::make_unique<Map>(tmxMap);

	batch = std::make_unique<SpriteBatch>();
	renderer = std::make_unique<GLTmxRenderer>(tmxMap, batch);

	playerTexture = std::make_unique<Texture>("assets/player.png");
	player = std::make_unique<Sprite>(playerTexture);

	const auto &spawnPoint = map->getSpawnPoint();
	playerPos.x = spawnPoint.x * map->getTileWidth();
	playerPos.y = spawnPoint.y * map->getTileHeight();

	return true;
}

void PlayPG::PlayPG::render(float deltaTime) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	renderer->renderAll(deltaTime);

	batch->begin();
	batch->draw(player, playerPos.x, playerPos.y);
	batch->end();

	SDL_GL_SwapWindow(window.get());
}
