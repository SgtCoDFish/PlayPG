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

#ifndef INCLUDE_COMPONENTS_RENDERABLE_HPP_
#define INCLUDE_COMPONENTS_RENDERABLE_HPP_

#include <memory>

#include <Ashley/Ashley.hpp>

namespace APG {
class SpriteBase;
class Sprite;
class AnimatedSprite;
class TmxRenderer;
}

namespace PlayPG {

enum class RenderableType {
	TILED, SPRITE, ANIMATION
};

class Renderable final : public ashley::Component {
public:
	explicit Renderable(APG::Sprite * const spriteToRender);
	explicit Renderable(APG::AnimatedSprite * const animSprite);
	explicit Renderable(APG::TmxRenderer * const renderer, unsigned int layerIndex);
	~Renderable() = default;

	RenderableType type;

	APG::SpriteBase * sprite = nullptr;
	APG::TmxRenderer * tmxRenderer = nullptr;
	unsigned int layerIndex = 0;

private:
};

}

#endif /* INCLUDE_COMPONENTS_RENDERABLE_HPP_ */