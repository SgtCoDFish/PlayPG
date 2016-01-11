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

#include <utility>
#include <memory>

#include "LoginServer.hpp"
#include "Character.hpp"
#include "net/packets/CharacterPackets.hpp"

#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

namespace PlayPG {

void LoginServer::processConnected() {
	static constexpr const double RUN_WAIT_TIME = 0.5;

	const auto logger = el::Loggers::getLogger("ServPG");

	auto start = std::chrono::high_resolution_clock::now();

	while (!done) {
		const auto now = std::chrono::high_resolution_clock::now();

		const auto timeSinceLastRun = std::chrono::duration_cast<std::chrono::seconds>(
		        (std::chrono::high_resolution_clock::now() - start)).count();

		if (timeSinceLastRun > RUN_WAIT_TIME) {
			start = now;

			std::lock_guard<std::mutex> sessionsLock(playerSessionMutex);

			// if true, we'll clear any nullptr sessions after we loop
			bool shouldClear = false;

			for (auto &session : playerSessions) {
				if (session->socket->hasError()) {
					shouldClear = true;

					// something went wrong, remove the socket.
					auto unIt = usernameToPlayerSession.find(session->username);

					if (unIt != usernameToPlayerSession.end()) {
						usernameToPlayerSession.erase(unIt);
					}

					session.reset();
					session = nullptr;
					continue;
				}

				if (session->socket->hasActivity()) {
					const auto opcodeBytes = session->socket->recv(2);

					if (opcodeBytes != 2) {
						logger->verbose(1, "Couldn't get opcode from socket with activity; got %v bytes.", opcodeBytes);
						session->socket->setError();
						continue;
					}

					const auto opcode = session->socket->getShort();

					switch (opcode) {
					case util::to_integral(ClientOpcode::REQUEST_CHARACTERS): {
						processCharacterRequest(session, logger);
						break;
					}

					default: {
						logger->verbose(8, "Unhandled opcode received: %v", opcode);
						break;
					}
					}
				}
			}

			if (shouldClear) {
				const int32_t sizeBefore = playerSessions.size();

				playerSessions.erase(
				        std::remove_if(playerSessions.begin(), playerSessions.end(),
				                [](const std::unique_ptr<PlayerSession> &ptr) {return ptr == nullptr;}),
				        playerSessions.end());

				const int32_t sizeAfter = playerSessions.size();

				logger->info("Purged %v connected sockets.", (sizeBefore - sizeAfter));
			}
		}
	}
}

void LoginServer::processCharacterRequest(const std::unique_ptr<PlayerSession> &session, el::Logger * const logger) {
	auto ps = std::unique_ptr<sql::PreparedStatement>(
	        mysqlConnection->prepareStatement("SELECT * FROM characters WHERE playerID=?;"));
	ps->setUInt(1, session->playerID);

	auto results = std::unique_ptr<sql::ResultSet>(ps->executeQuery());

	std::vector<Character> characters;

	if (results->rowsCount() > 0) {
		while (results->next()) {
			const std::string name = results->getString("name");

			const auto maxHP = results->getInt("maxHP");
			const auto strength = results->getInt("strength");
			const auto intelligence = results->getInt("intelligence");

			characters.emplace_back(name, maxHP, strength, intelligence);
		}
	}

	PlayerCharacters pc(std::move(characters));

	session->socket->clear();
	session->socket->put(&pc.buffer);

	auto charBytesSent = session->socket->send();

	if (charBytesSent == 0) {
		logger->error("Couldn't send character response.");

		session->socket->disconnect();
		session->socket->setError();

		return;
	}
}

}
