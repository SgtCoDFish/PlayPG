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

#include <typeinfo>

#include <APG/APGInput.hpp>

#include "systems/InputSystem.hpp"
#include "systems/MovementSystem.hpp"

namespace PlayPG {

InputSystem::InputSystem(APG::InputManager * const inputManager_, MovementSystem * const movementSystem, int64_t priority) :
		        IteratingSystem(ashley::Family::getFor( { typeid(KeyboardInputListener) }), priority),
		        inputManager { inputManager_ },
		        movementSystem_ { movementSystem } {

}

void InputSystem::processEntity(ashley::Entity * const entity, float deltaTime) {
	if (inputManager->isKeyJustPressed(SDL_SCANCODE_W) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_8)) {
		doMove(entity, deltaTime, 0, -1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_A) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_4)) {
		doMove(entity, deltaTime, -1, 0);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_S) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_2)) {
		doMove(entity, deltaTime, 0, 1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_D) || inputManager->isKeyJustPressed(SDL_SCANCODE_KP_6)) {
		doMove(entity, deltaTime, 1, 0);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_7)) {
		doMove(entity, deltaTime, -1, -1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_9)) {
		doMove(entity, deltaTime, 1, -1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_1)) {
		doMove(entity, deltaTime, -1, 1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_KP_3)) {
		doMove(entity, deltaTime, 1, 1);
	} else if (inputManager->isKeyJustPressed(SDL_SCANCODE_SPACE)) {
		doInteraction(entity, deltaTime);
	}
}

void InputSystem::doMove(ashley::Entity * const entity, float deltaTime, int32_t xTiles, int32_t yTiles) {
	movementSystem_->addMove(entity, xTiles, yTiles);
}

void InputSystem::doInteraction(ashley::Entity * const entity, float deltaTime) {

}

}
