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
#ifndef INCLUDE_SYSTEMS_MOVEMENTSYSTEM_HPP_
#define INCLUDE_SYSTEMS_MOVEMENTSYSTEM_HPP_

#include <cstdint>

#include <vector>

#include <Ashley/Ashley.hpp>

namespace Tmx {
class Map;
}

namespace PlayPG {
class Map;

struct Move {
	explicit Move(ashley::Entity * entity_, int32_t xTiles_, int32_t yTiles_) :
			        entity { entity_ },
			        xTiles { xTiles_ },
			        yTiles { yTiles_ } {
	}

	~Move() = default;

	ashley::Entity * const entity;
	const int32_t xTiles;
	const int32_t yTiles;
};

class MovementSystem : public ashley::EntitySystem {
public:
	explicit MovementSystem(Map * const map, int64_t priority);

	virtual ~MovementSystem() = default;

	void update(float deltaTime) override final;

	void addMove(Move &&move);
	void addMove(ashley::Entity * entity, int32_t xTiles, int32_t yTiles);

	Map * getMap() const {
		return map_;
	}

private:
	Map * map_;

	std::vector<Move> moves_;
};

}

#endif /* INCLUDE_SYSTEMS_MOVEMENTSYSTEM_HPP_ */
