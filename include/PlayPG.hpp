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

#ifndef INCLUDE_PLAYPG_HPP_
#define INCLUDE_PLAYPG_HPP_

#include <vector>
#include <utility>

#include <tmxparser/Tmx.h>

#include <glm/vec2.hpp>

#include <APG/APG.hpp>

#include <Ashley/Ashley.hpp>

#include "Map.hpp"
#include "data/Character.hpp"

namespace ashley {
class Entity;
}

namespace PlayPG {

class PlayPG final : public APG::SDLGame {
public:
	static el::Logger *logger;

	explicit PlayPG();
	virtual ~PlayPG() = default;

	bool init() override;
	void render(float deltaTime) override;

private:
	void doLogin();

	std::unique_ptr<APG::Camera> camera;
	std::unique_ptr<APG::SpriteBatch> batch;

	std::unique_ptr<Map> mapOutdoor;
	std::unique_ptr<Map> mapIndoor;
	bool indoor = false;

	std::unique_ptr<APG::Texture> playerTexture;
	std::unique_ptr<APG::Sprite> playerSprite;

	std::unique_ptr<APG::GLTmxRenderer> outdoorRenderer;
	std::unique_ptr<APG::GLTmxRenderer> indoorRenderer;

	std::unique_ptr<ashley::Engine> engine;

	ashley::Entity *player = nullptr;
	void changeToWorld(const std::unique_ptr<Map> &renderer);

	std::unique_ptr<Character> currentCharacter;

	static constexpr const char * const addr = "localhost";
	static constexpr const int port = 10419;

	APG::SDLSocket socket;
};

}

#endif /* INCLUDE_PLAYPG_HPP_ */
