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

#include <cstdint>
#include <cstring>

#include <utility>
#include <string>
#include <sstream>

#include <Ashley/Ashley.hpp>

#include <APG/APG.hpp>

#include <rapidjson/writer.h>

#include "Components.hpp"
#include "Systems.hpp"

#include "PlayPG.hpp"
#include "PlayPGVersion.hpp"
#include "Map.hpp"
#include "ClientMapUtil.hpp"

#include "net/packets/LoginPackets.hpp"

using namespace APG;

namespace PlayPG {

el::Logger *PlayPG::logger = nullptr;

PlayPG::PlayPG(int argc, char *argv[]) :
		        APG::SDLGame("PlayPG", 1280u, 720u, 4, 5),
		        addr { argc > 1 ? argv[1] : "localhost" },
		        socket { addr, port } {
	PlayPG::logger = el::Loggers::getLogger("PlayPG");

	parseCommandLineArgs(argc, argv);
}

bool PlayPG::init() {
	camera = std::make_unique<Camera>(screenWidth, screenHeight);
	camera->setToOrtho(false, screenWidth, screenHeight);
	batch = std::make_unique<SpriteBatch>();

	outdoorRenderer = std::make_unique<GLTmxRenderer>("assets/outdoor.tmx", batch.get());
	mapOutdoor = std::make_unique<Map>(outdoorRenderer->getMap());

	indoorRenderer = std::make_unique<GLTmxRenderer>("assets/room1.tmx", batch.get());
	mapIndoor = std::make_unique<Map>(indoorRenderer->getMap());

	playerTexture = std::make_unique<Texture>("assets/player.png");
	playerSprite = std::make_unique<Sprite>(playerTexture);

	engine = std::make_unique<ashley::Engine>();

	auto movementSystem = engine->addSystem<MovementSystem>(mapOutdoor.get(), 6000);

	engine->addSystem<InputSystem>(inputManager.get(), movementSystem, 5000);
	engine->addSystem<CameraFocusSystem>(camera.get(), 7500);
	engine->addSystem<RenderSystem>(batch.get(), camera.get(), 10000);

	changeToWorld(mapOutdoor);

	return true;
}

bool PlayPG::doLogin() {
	const auto logger = el::Loggers::getLogger("PlayPG");
	if (!socket.isConnected()) {
		socket.connect();

		if (socket.hasError()) {
			return false;
		}

		if (!socket.waitForActivity(2000)) {
			logger->error("Server didn't respond in time.");
			return false;
		}

		int read = socket.recv();

		const auto opcode = socket.getShort();

		logger->info("Got %v bytes, opcode %v.", read, opcode);

		if (opcode != static_cast<opcode_type_t>(ServerOpcode::LOGIN_AUTHENTICATION_CHALLENGE)) {
			logger->info("Got opcode %v which doesn't match the expected value. Ignoring.");
			socket.clear();
			return false;
		}

		const uint16_t jsonSize = socket.getShort();

		auto json = socket.getStringByLength(jsonSize);

//		logger->info("JSON: %v", json);

		APG::JSONSerializer<AuthenticationChallenge> challengeS11N;
		const auto challenge = challengeS11N.fromJSON(json.c_str());

		logger->info("Got challenge from \"%v\", v%v (%v).", challenge.name, challenge.version, challenge.versionHash);
//		logger->info("Got pubkey: %v", challenge.pubKey);

		serverPubKey = challenge.pubKey;

		crypto = std::make_unique<RSACrypto>(serverPubKey, true);

		socket.clear();

		if (std::strcmp(challenge.version.c_str(), Version::versionString) != 0
		        || std::strcmp(challenge.versionHash.c_str(), Version::gitHash) != 0) {
			logger->info("Version check failed.");

			VersionMismatch mismatchPacket;

			socket.put(&mismatchPacket.buffer);
			socket.send();

			return false;
		}

		logger->info("Version check successful.");
	} else {
		socket.clear();
	}

	const auto encPass = crypto->encryptStringPublic("testa");
	logger->info("Sending %v byte password", encPass.size());

	AuthenticationIdentity identity(username, encPass);

	socket.put(&identity.buffer);
	const auto sentAuthDetailBytes = socket.send();
	logger->info("Sent %v auth detail bytes, opcode %v.", sentAuthDetailBytes, (opcode_type_t) identity.opcode);

	socket.clear();

	socket.waitForActivity(2000);

	socket.recv();

	const auto respOpcode = socket.getShort();

	if (respOpcode == static_cast<opcode_type_t>(ServerOpcode::LOGIN_AUTHENTICATION_RESPONSE)) {
		const auto respSize = socket.getShort();
		const auto respJSON = socket.getStringByLength(respSize);
		logger->info("Got authentication response: %v", respJSON);

		APG::JSONSerializer<AuthenticationResponse> responseS11N;
		const auto response = responseS11N.fromJSON(respJSON.c_str());

		if (!response.successful) {
			logger->info("Authentication failed with username %v: %v", username, response.message);

			if (response.attemptsRemaining == 0) {
				socket.disconnect();
			}

			return false;
		}
	} else {
		return false;
	}

	return true;
}

void PlayPG::changeToWorld(const std::unique_ptr<Map> &map) {
	engine->removeAllEntities();

	{
		auto frontLayerEntities = MapUtil::generateFrontLayerEntities(*map, outdoorRenderer.get());
		logger->info("Generated %v front layer entities.", frontLayerEntities.size());

		for (auto &ent : frontLayerEntities) {
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
		auto backLayerEntities = MapUtil::generateBackLayerEntities(*map, outdoorRenderer.get());
		logger->info("Generated %v back layer entities.", backLayerEntities.size());

		for (auto &ent : backLayerEntities) {
			engine->addEntity(std::move(ent));
		}
	}
}

void PlayPG::render(float deltaTime) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	switch (gameState) {
	case GameState::LOGIN: {
		if (inputManager->isKeyJustPressed(SDL_SCANCODE_SPACE)) {
			if (doLogin()) {
				gameState = GameState::PLAYING;
			}
		} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_1)) {
			username = "SgtCoDFish@example.com";
		}
		break;
	}

	case GameState::PLAYING: {
		if (inputManager->isKeyJustPressed(SDL_SCANCODE_RETURN)) {
			if (indoor) {
				indoor = false;
				changeToWorld(mapOutdoor);
			} else {
				indoor = true;
				changeToWorld(mapIndoor);
			}
		}

		outdoorRenderer->update(deltaTime);

		engine->update(deltaTime);
		break;
	}
	}

	SDL_GL_SwapWindow(window.get());
}

void PlayPG::parseCommandLineArgs(int argc, char *argv[]) {
	// no-op
}

}
