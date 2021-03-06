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

#include <vector>
#include <utility>

#include <Ashley/Ashley.hpp>

#include <tmxparser/Tmx.h>

#include "components/Position.hpp"
#include "systems/MovementSystem.hpp"
#include "net/Packet.hpp"
#include "net/packets/GameplayPackets.hpp"
#include "systems/NetworkDispatchSystem.hpp"
#include "Map.hpp"

namespace PlayPG {

MovementSystem::MovementSystem(Map * const map, int64_t priority) :
		        MovementSystem(map, nullptr, priority) {
}

MovementSystem::MovementSystem(Map * const map, NetworkDispatchSystem * const networkDispatchSystem, int64_t priority) :
		        EntitySystem(priority),
		        map_ { map } {
	attachNetworkingSystem(networkDispatchSystem);
}

void MovementSystem::update(float deltaTime) {
	const auto mapper = ashley::ComponentMapper<Position>::getMapper();

	for (const auto &move : moves_) {
		const auto &position = mapper.get(move.entity);

		const glm::vec2 destination = { position->p.x + move.xTiles * map_->getTileWidth(), position->p.y
		        + move.yTiles * map_->getTileHeight() };

		if (!map_->isSolidAtCoord(destination)) {
			position->p = destination;
		}

		if(networkDispatchSystem_ != nullptr) {
			networkDispatchSystem_->queuePacket(MovementPacket(move));
		}
	}

	moves_.clear();
}

MovementSystem &MovementSystem::attachNetworkingSystem(NetworkDispatchSystem * const networkDispatchSystem) {
	networkDispatchSystem_ = networkDispatchSystem;

	return *this;
}

MovementSystem &MovementSystem::detachNetworkingSystem() {
	networkDispatchSystem_ = nullptr;

	return *this;
}

void MovementSystem::addMove(Move &&move) {
	moves_.emplace_back(std::move(move));
}

void MovementSystem::addMove(ashley::Entity * entity, int32_t xTiles, int32_t yTiles) {
	moves_.emplace_back(entity, xTiles, yTiles);
}

}
