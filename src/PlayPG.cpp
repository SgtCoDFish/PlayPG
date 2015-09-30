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

#include <APG/APG.hpp>

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
//	indoorTMXMap->ParseFile("assets/room1.tmx");

	playerTexture = std::make_unique<Texture>("assets/player.png");
	playerSprite = std::make_unique<Sprite>(playerTexture);

	engine = std::make_unique<ashley::Engine>();
	engine->addSystem<RenderSystem>(batch.get(), 10000);

	layer = engine->addEntity();

	layer->add<Position>();
	layer->add<Renderable>(tmxRenderer.get(), 0u);

	player = engine->addEntity();

	player->add<Position>(32, 32);
	player->add<Renderable>(playerSprite.get());

	return true;
}

void PlayPG::handleInput() {
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
	}
}

void PlayPG::doMove(int32_t xTiles, int32_t yTiles) {

}

void PlayPG::doInteraction() {

}

void PlayPG::render(float deltaTime) {
	handleInput();

	const auto &positionComponent = ashley::ComponentMapper<Position>::getMapper().get(player);
	camera->position.x = positionComponent->p.x - screenWidth / 2.0f;
	camera->position.y = positionComponent->p.y - screenHeight / 2.0f;

	camera->update();

	batch->setProjectionMatrix(camera->combinedMatrix);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	engine->update(deltaTime);

	SDL_GL_SwapWindow(window.get());
}

}
