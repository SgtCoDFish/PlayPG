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

#include <Ashley/Ashley.hpp>

#include <APG/APGCore.hpp>
#include <APG/APGGraphics.hpp>

#include "components/Position.hpp"
#include "components/FocalPoint.hpp"

#include "systems/CameraFocusSystem.hpp"

namespace PlayPG {

CameraFocusSystem::CameraFocusSystem(APG::Camera * const camera_, int64_t priority_) :
		        IteratingSystem(ashley::Family::getFor( { typeid(Position), typeid(FocalPoint) }), priority_),
		        camera { camera_ } {

}

void CameraFocusSystem::processEntity(ashley::Entity * const &entity, float deltaTime) {
	const auto &position = ashley::ComponentMapper<Position>::getMapper().get(entity)->p;

	camera->position.x = position.x - APG::Game::screenWidth / 2.0f;
	camera->position.y = position.y - APG::Game::screenHeight / 2.0f;

	camera->update();
}

}
