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

#include <Ashley/Ashley.hpp>

#include <APG/APG.hpp>

#include "components/Renderable.hpp"
#include "components/Position.hpp"
#include "systems/RenderSystem.hpp"

namespace PlayPG {

RenderSystem::RenderSystem(APG::SpriteBatch * const targetBatch, APG::Camera * const camera_, int64_t priority) :
		        IteratingSystem(ashley::Family::getFor( { typeid(Renderable), typeid(Position) }), priority),
		        batch { targetBatch },
		        camera { camera_ } {
}

void RenderSystem::processEntity(ashley::Entity * const entity, float deltaTime) {
	const auto positionMapper = ashley::ComponentMapper<Position>::getMapper();
	const auto renderableMapper = ashley::ComponentMapper<Renderable>::getMapper();

	const auto position = positionMapper.get(entity);
	const auto renderable = renderableMapper.get(entity);

	switch (renderable->type) {
	case RenderableType::ANIMATION: {
		static_cast<APG::AnimatedSprite*>(renderable->sprite)->update(deltaTime);
		drawSpriteBase(position, renderable);
		break;
	}

	case RenderableType::SPRITE: {
		drawSpriteBase(position, renderable);
		break;
	}

	case RenderableType::TILED: {
		drawTiled(position, renderable);
	}
	}
}

void RenderSystem::update(float deltaTime) {
	batch->setProjectionMatrix(camera->combinedMatrix);

	batch->begin();

	IteratingSystem::update(deltaTime);

	batch->end();
}

void RenderSystem::drawSpriteBase(Position * const position, Renderable * const renderable) {
	batch->draw(renderable->sprite, position->p.x, position->p.y);
}

void RenderSystem::drawTiled(Position * const position, Renderable * const renderable) {
	const auto layer = renderable->tmxRenderer->getMap()->GetTileLayer(renderable->layerIndex);
	renderable->tmxRenderer->renderLayer(layer);
}

}
