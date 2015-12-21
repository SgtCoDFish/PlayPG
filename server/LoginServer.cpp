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

#include <APG/core/APGeasylogging.hpp>

#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <SDL2/SDL.h>

#include "server/LoginServer.hpp"
#include "net/Opcodes.hpp"
#include "net/packets/LoginPackets.hpp"
#include "PlayPGVersion.hpp"

namespace PlayPG {

static const std::unordered_map<opcode_type_t, OpcodeDetails> acceptedOpcodes_login = { //
                { static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY), OpcodeDetails(
                        "Login request", true) }, //
        };

LoginServer::LoginServer(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_) :
		        Server(serverDetails_, databaseDetails_, acceptedOpcodes_login) {
	playerAcceptor = std::make_unique<APG::SDLAcceptorSocket>(serverDetails.port, true);
}

void LoginServer::run() {
	auto logger = el::Loggers::getLogger("ServPG");

	logger->info("Running login server on port %v.", serverDetails.port);
	logger->info("\"%v\", version %v (%v)", serverDetails.friendlyName, Version::versionString, Version::gitHash);

	while (true) {
		auto newPlayerSocket = playerAcceptor->acceptSocket();

		if (newPlayerSocket != nullptr) {
			logger->info("Accepted a connection from: %v", newPlayerSocket->remoteHost);

//			auto ps = std::unique_ptr<sql::Statement>(mysqlConnection->createStatement());
//
//			auto res = std::unique_ptr<sql::ResultSet>(ps->executeQuery("SELECT * FROM players;"));
//
//			while (res->next()) {
//				std::cout << "id = " << res->getInt("id") << "\nname = " << res->getString("email") << std::endl;
//			}

			AuthenticationChallenge challenge(Version::versionString, Version::gitHash,
			        this->serverDetails.friendlyName);

			newPlayerSocket->put(&challenge.buffer);
			newPlayerSocket->send();

			newPlayerSocket->clear();

			SDL_Delay(500);

			const auto receivedCount = newPlayerSocket->recv();

			if (receivedCount <= 0) {
				logger->info("Got no response from new socket, closing.");
				continue;
			}

			const opcode_type_t opcode = newPlayerSocket->getShort();

			if (opcode != static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY)) {
				if (opcode == static_cast<opcode_type_t>(ClientOpcode::VERSION_MISMATCH)) {
					logger->info("Client has mismatched version, closing.");
				} else {
					logger->info("Client sent unexpected input, closing.");
				}

				continue;
			}

			logger->info("Client attempting to authenticate.");

			const auto authIDJSONSize = newPlayerSocket->getShort();

			APG::JSONSerializer<AuthenticationIdentity> authIDS11N;
			auto authIDBuffer = std::make_unique<char[]>(authIDJSONSize);
			newPlayerSocket->getBytes(reinterpret_cast<uint8_t *>(authIDBuffer.get()), authIDJSONSize);

			std::string jsonString(authIDBuffer.get(), authIDJSONSize);
			AuthenticationIdentity id = authIDS11N.fromJSON(jsonString.c_str());

			logger->verbose(9, "Username: %v - Password: %v", id.username, id.password);

			auto newSession = std::make_unique<PlayerSession>(id.username, std::move(newPlayerSocket));
			logger->verbose(1, "Storing new user \"%v\" into connection pool with GUID %v.", newSession->username, newSession->guid);
			// new connection established so keep for later.
			playerSessions.emplace_back(std::move(newSession));
		}
	}
}

}
