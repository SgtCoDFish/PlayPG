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

#include <Ashley/Ashley.hpp>

#include <APG/APG.hpp>

#include "Components.hpp"
#include "Systems.hpp"

#include "PlayPG.hpp"
#include "Map.hpp"

using namespace APG;

namespace PlayPG {

el::Logger *PlayPG::logger = nullptr;

PlayPG::PlayPG() :
		        APG::SDLGame("PlayPG", 1280u, 720u, 4, 5) {
	PlayPG::logger = el::Loggers::getLogger("PlayPG");
}

bool PlayPG::init() {
	camera = std::make_unique<Camera>(screenWidth, screenHeight);
	camera->setToOrtho(false, screenWidth, screenHeight);
	batch = std::make_unique<SpriteBatch>();

	tmxRenderer = std::make_unique<GLTmxRenderer>("assets/outdoor.tmx", batch.get());
	map = std::make_unique<Map>(tmxRenderer.get());
//	indoorTMXMap->ParseFile("assets/room1.tmx");

	playerTexture = std::make_unique<Texture>("assets/player.png");
	playerSprite = std::make_unique<Sprite>(playerTexture);

	engine = std::make_unique<ashley::Engine>();
	engine->addSystem<InputSystem>(inputManager.get(), 5000);
	engine->addSystem<CameraFocusSystem>(camera.get(), 7500);
	engine->addSystem<RenderSystem>(batch.get(), camera.get(), 10000);

	{
		auto frontLayerEntities = map->generateFrontLayerEntities();
		logger->info("Generated %v front layer entities.", frontLayerEntities.size());

		for(auto &ent : frontLayerEntities) {
			engine->addEntity(std::move(ent));
		}
	}

	player = engine->addEntity();

	const auto spawnPoint = map->getSpawnPoint() * glm::ivec2(map->getTileWidth(), map->getTileHeight());
	player->add<Position>(spawnPoint.x, spawnPoint.y);
	player->add<Renderable>(playerSprite.get());
	player->add<KeyboardInputListener>();
	player->add<FocalPoint>();

	{
		auto backLayerEntities = map->generateBackLayerEntities();
		logger->info("Generated %v back layer entities.", backLayerEntities.size());

		for(auto &ent : backLayerEntities) {
			engine->addEntity(std::move(ent));
		}
	}

	return true;
}

void PlayPG::doInteraction() {

}

void PlayPG::render(float deltaTime) {
	batch->setProjectionMatrix(camera->combinedMatrix);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	tmxRenderer->update(deltaTime);

	engine->update(deltaTime);

	SDL_GL_SwapWindow(window.get());
}

}
