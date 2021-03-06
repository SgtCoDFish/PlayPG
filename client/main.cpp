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

#include <cstdlib>
#include <cstdint>

#include <chrono>

#include <APG/APG.hpp>
INITIALIZE_EASYLOGGINGPP

#include "PlayPG.hpp"

int main(int argc, char *argv[]) {
	START_EASYLOGGINGPP(argc, argv);
	APG::Game::setLoggerToAPGStyle("PlayPG");

	{
		auto playPG = std::make_unique<PlayPG::PlayPG>(argc, argv);

		if (!playPG->init()) {
			return EXIT_FAILURE;
		}

		bool done = false;

		auto startTime = std::chrono::high_resolution_clock::now();

		while (!done) {
			auto timeNow = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - startTime).count()
			        / 1000.0f;

			startTime = timeNow;

			done = playPG->update(deltaTime);
		}
	}
	return EXIT_SUCCESS;
}
