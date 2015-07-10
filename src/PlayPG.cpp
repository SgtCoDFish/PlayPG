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
	camera = std::make_unique<Camera>();
	camera->setToOrtho(false, screenWidth, screenHeight);
	batch = std::make_unique<SpriteBatch>();

	indoorTMXMap = std::make_unique<Tmx::Map>();
	indoorTMXMap->ParseFile("assets/room1.tmx");

	if (indoorTMXMap->HasError()) {
		logger->fatal("Couldn't load room1.tmx: %v", indoorTMXMap->GetErrorText());
		return false;
	}

	indoorMap = std::make_unique<Map>(indoorTMXMap);

	outdoorTMXMap = std::make_unique<Tmx::Map>();
	outdoorTMXMap->ParseFile("assets/outdoor.tmx");

	if (outdoorTMXMap->HasError()) {
		logger->fatal("Couldn't load room1.tmx: %v", outdoorTMXMap->GetErrorText());
		return false;
	}

	outdoorMap = std::make_unique<Map>(outdoorTMXMap);

	indoorRenderer = std::make_unique<GLTmxRenderer>(indoorTMXMap, batch);
	outdoorRenderer = std::make_unique<GLTmxRenderer>(outdoorTMXMap, batch);

	currentRenderer = outdoorRenderer.get();
	currentMap = outdoorMap.get();

	playerTexture = std::make_unique<Texture>("assets/player.png");
	player = std::make_unique<Sprite>(playerTexture);

	const auto &spawnPoint = outdoorMap->getSpawnPoint();
	playerPos.x = spawnPoint.x * outdoorMap->getTileWidth();
	playerPos.y = spawnPoint.y * outdoorMap->getTileHeight();

	return true;
}

void PlayPG::PlayPG::handleInput() {
	if (inputManager->isKeyJustPressed(SDL_SCANCODE_W) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_8)) {
		doMove(0, -1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_A) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_4)) {
		doMove(-1, 0);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_S) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_2)) {
		doMove(0, 1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_D) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_6)) {
		doMove(1, 0);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_7)) {
		doMove(-1, -1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_9)) {
		doMove(1, -1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_1)) {
		doMove(-1, 1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_3)) {
		doMove(1, 1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_SPACE)) {
		doInteraction();
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_RETURN)) {
		if (currentRenderer == outdoorRenderer.get()) {
			currentRenderer = indoorRenderer.get();
			currentMap = indoorMap.get();

			const auto &spawnPoint = indoorMap->getSpawnPoint();
			playerPos.x = spawnPoint.x * indoorMap->getTileWidth();
			playerPos.y = spawnPoint.y * indoorMap->getTileHeight();
		} else {
			currentRenderer = outdoorRenderer.get();
			currentMap = outdoorMap.get();

			const auto &spawnPoint = outdoorMap->getSpawnPoint();
			playerPos.x = spawnPoint.x * outdoorMap->getTileWidth();
			playerPos.y = spawnPoint.y * outdoorMap->getTileHeight();
		}
	}
}

void PlayPG::PlayPG::doMove(int32_t xTiles, int32_t yTiles) {
	const int tileWidth = currentMap->getTileWidth(), tileHeight = currentMap->getTileHeight();

	const float xDest = playerPos.x / tileWidth + xTiles;
	const float yDest = playerPos.y / tileHeight + yTiles;

	const bool solid = currentMap->isSolid(xDest, yDest);

	logger->info("Moving (%v, %v) -> (%v, %v) (%v blocked).", playerPos.x, playerPos.y, xDest, yDest,
	        (solid ? "is" : "not"));

	if (!solid) {
		playerPos.x = xDest * tileWidth;
		playerPos.y = yDest * tileHeight;
	}
}

void PlayPG::PlayPG::doInteraction() {
	const float xTile = playerPos.x / currentMap->getTileWidth();
	const float yTile = playerPos.y / currentMap->getTileHeight();

	if (currentMap->isInteresting(xTile - 1, yTile + 0)) {
		logger->info("Interesting to the west...");
	} else if (currentMap->isInteresting(xTile - 1, yTile - 1)) {
		logger->info("Interesting to the north-west...");
	} else if (currentMap->isInteresting(xTile + 0, yTile - 1)) {
		logger->info("Interesting to the north...");
	} else if (currentMap->isInteresting(xTile + 1, yTile - 1)) {
		logger->info("Interesting to the north-east...");
	} else if (currentMap->isInteresting(xTile + 1, yTile + 0)) {
		logger->info("Interesting to the east...");
	} else if (currentMap->isInteresting(xTile + 1, yTile + 1)) {
		logger->info("Interesting to the south-east...");
	} else if (currentMap->isInteresting(xTile + 0, yTile + 1)) {
		logger->info("Interesting to the south...");
	} else if (currentMap->isInteresting(xTile - 1, yTile + 1)) {
		logger->info("Interesting to the south-west...");
	} else if (currentMap->isInteresting(xTile + 0, yTile + 0)) {
		logger->info("Interesting right here...");
	}
}

void PlayPG::PlayPG::render(float deltaTime) {
	handleInput();

	camera->position.x = playerPos.x - screenWidth / 2.0f;
	camera->position.y = playerPos.y - screenHeight / 2.0f;

	camera->update();

	batch->setProjectionMatrix(camera->combinedMatrix);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	currentRenderer->renderAll(deltaTime);

	batch->begin();
	batch->draw(player, playerPos.x, playerPos.y);
	batch->end();

	SDL_GL_SwapWindow(window.get());
}
