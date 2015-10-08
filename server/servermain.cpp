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

#include <cstdlib>

#include <memory>

#include <APG/APG.hpp>
INITIALIZE_EASYLOGGINGPP

#include "Systems.hpp"
#include "net/Packet.hpp"

int main(int argc, char *argv[]) {
	START_EASYLOGGINGPP(argc, argv);
	APG::Game::setLoggerToAPGStyle("ServPG");
	auto logger = el::Loggers::getLogger("ServPG");
	logger->info("ServPG started.");

	APG::SDLGame::initialiseSDL();

	auto inputManager = std::make_unique<APG::SDLInputManager>();

	IPaddress ip;
	TCPsocket socket;

	if (SDLNet_ResolveHost(&ip, nullptr, 10419) == -1) {
		logger->fatal("Couldn't resolve host: %v.", SDLNet_GetError());
		return EXIT_FAILURE;
	}

	socket = SDLNet_TCP_Open(&ip);

	if (socket == nullptr) {
		logger->fatal("Couldn't open server socket: %v.", SDLNet_GetError());
		return EXIT_FAILURE;
	}

	logger->info("Successfully listening on port 10419.");

	TCPsocket accepted;

	while ((accepted = SDLNet_TCP_Accept(socket)) == nullptr) {
	}

	logger->info("Accepted a connection!");

	while (true) {
		PlayPG::Packet packet;

		if(SDLNet_TCP_Recv(accepted, &packet, sizeof(PlayPG::Packet)) <= 0) {
			logger->info("Finished with remote host.");
			break;
		}

		logger->info("Received packet #%v, entity #%v", packet.id, packet.entity);
	}

	APG::SDLGame::shutdownSDL();
	return EXIT_SUCCESS;
}
