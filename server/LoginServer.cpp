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
 
#include "LoginServer.hpp"

#include <APG/core/APGeasylogging.hpp>

#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

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
	playerAcceptor = getAcceptorSocket(serverDetails_.port, true);
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

			if (!newPlayerSocket->waitForActivity(3000u)) {
				logger->warn("Client timed out when responding to challenge. Abandoning.");
				continue;
			}

			logger->info("Got %v response bytes.", newPlayerSocket->recv());

			const opcode_type_t opcode = newPlayerSocket->getUInt16();

			if (opcode != static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY)) {
				if (opcode == static_cast<opcode_type_t>(ClientOpcode::VERSION_MISMATCH)) {
					logger->info("Client has mismatched version, closing.");
				} else {
					logger->info("Client sent unexpected input, closing.");
				}

				continue;
			}

			const auto authIDJSONSize = newPlayerSocket->getUInt16();
			newPlayerSocket->recv(authIDJSONSize);

			APG::JSONSerializer<AuthenticationIdentity> authIDS11N;
			auto jsonString = newPlayerSocket->getStringByLength(authIDJSONSize);

			logger->info("Client attempting to authenticate with JSON: %v", jsonString);

			AuthenticationIdentity id = authIDS11N.fromJSON(jsonString.c_str());

			auto newSession = std::make_unique<PlayerSession>(id.username, std::move(newPlayerSocket));
			logger->verbose(9, "New user session for \"%v\": GUID %v.", newSession->username, newSession->guid);
			// new connection established so keep for later.
			playerSessions.emplace_back(std::move(newSession));
		} else {
			if(playerAcceptor->hasError()) {
				logger->error("Error in playerAcceptor, exiting.");
				break;
			}
		}
	}
}

}
