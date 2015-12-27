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

// Included first to handle windows weirdness with includes.
#include "LoginServer.hpp"

#include <chrono>
#include <algorithm>

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

//static const std::unordered_map<opcode_type_t, OpcodeDetails> acceptedOpcodes_login = { //
//        { static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY), //
//                OpcodeDetails("Login request", true) }, //
//                { static_cast<opcode_type_t>(ClientOpcode::VERSION_MISMATCH), //
//                        OpcodeDetails("The client's version doesn't match the server's.", false) }, };

LoginServer::LoginServer(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_) :
		        Server(serverDetails_, databaseDetails_) {
	playerAcceptor = getAcceptorSocket(serverDetails_.port, true);
}

void LoginServer::run() {
	auto logger = el::Loggers::getLogger("ServPG");

	processingThread = std::thread([this]() {this->processIncoming();});

	logger->info("Running login server on port %v.", serverDetails.port);
	logger->info("\"%v\", version %v (%v)", serverDetails.friendlyName, Version::versionString, Version::gitHash);

//	const auto encStr = crypto.encryptStringPublic(Version::gitHash);
//	const auto decStr = crypto.decryptStringPrivate(encStr);
//	logger->info("ENC: %v", encStr);
//	logger->info("DEC: %v", decStr);

	while (!done) {
		auto newPlayerSocket = playerAcceptor->acceptSocket();

		if (newPlayerSocket != nullptr) {
			logger->verbose(9, "Accepted a connection from: %v. Sending to processing thread.",
			        newPlayerSocket->remoteHost);

			std::lock_guard<std::mutex> incomingGuard(incomingConnectionsMutex);

			incomingConnections.emplace_back(std::move(newPlayerSocket));
		} else {
			if (playerAcceptor->hasError()) {
				logger->error("Error in playerAcceptor, exiting.");
				break;
			}
		}
	}

	processingThread.join();
}

void LoginServer::processIncoming() {
	static constexpr const double DONE_PURGE_TIME = 5.0;

	auto logger = el::Loggers::getLogger("ServPG");
	AuthenticationChallenge challenge(Version::versionString, Version::gitHash, serverDetails.friendlyName, crypto.getPublicKeyPEM());

	auto start = std::chrono::high_resolution_clock::now();

	while (!done) {
		std::lock_guard<std::mutex> lg(incomingConnectionsMutex);

		for (auto &connection : incomingConnections) {
			if (connection.state != IncomingConnectionState::DONE) {
				if (connection.socket->hasError()) {
					connection.state = IncomingConnectionState::DONE;
				}
			}

			switch (connection.state) {
			case (IncomingConnectionState::FRESH): {
				processFreshSocket(connection, challenge, logger);
				break;
			}

			case (IncomingConnectionState::CHALLENGE_SENT): {
				processChallengeSentSocket(connection, logger);
				break;
			}

			case (IncomingConnectionState::LOGIN_FAILED): {
				processLoginFailedSocket(connection, logger);
				break;
			}
			case (IncomingConnectionState::DONE): {
				processDoneSocket(connection, logger);
				break;
			}
			}
		}

		const auto timeSinceLastPurge = std::chrono::duration_cast<std::chrono::seconds>(
		        (std::chrono::high_resolution_clock::now() - start)).count();

		if (timeSinceLastPurge > DONE_PURGE_TIME) {
			const auto sizeBefore = incomingConnections.size();

			incomingConnections.erase(
			        std::remove_if(incomingConnections.begin(), incomingConnections.end(),
			                [](const IncomingConnection &ic) {return ic.state == IncomingConnectionState::DONE;}),
			        incomingConnections.end());

			const auto removed = sizeBefore - incomingConnections.size();

			if (removed > 0) {
				logger->info("Purged %v incoming sockets.", removed);
			}

			start = std::chrono::high_resolution_clock::now();
		}
	}
}

void LoginServer::processFreshSocket(IncomingConnection &connection, AuthenticationChallenge &challenge,
        el::Logger * const logger) {
	/*
	 * Fresh connections need an auth challenge sending and nothing else.
	 */

	logger->verbose(9, "Got fresh socket.");

	connection.socket->put(&challenge.buffer);
	challenge.buffer.setReadPos(0);

	connection.socket->send();
	connection.socket->clear();

	if (connection.socket->hasError()) {
		logger->verbose(9, "Socket entered error state for fresh connection, dropping.");
		connection.state = IncomingConnectionState::DONE;
	} else {
		connection.state = IncomingConnectionState::CHALLENGE_SENT;
	}
}

void LoginServer::processChallengeSentSocket(IncomingConnection &connection, el::Logger * const logger) {
	/*
	 * After a challenge is sent, the remote host is expected to identify themselves.
	 * Either they send:
	 * - VersionMismatch which we can't really help with so we close.
	 * - AuthenticationIdentity (correct): they're done and sent to a map server.
	 * - AuthenticationIdentity (incorrect): they lose a login attempt and get another
	 * 										 go/disconnected depending on their attempts used
	 * - Something else: they lose a login attempt and go again/get disconnected
	 */

	if (!connection.socket->hasActivity()) {
		return;
	}

	logger->verbose(9, "Handling CHALLENGE_SENT.");

	logger->info("Read %v bytes", connection.socket->recv(4096));
	const auto opcode = connection.socket->getShort();

	if (opcode == static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY)) {
		// attempt auth
		const auto jsonLength = connection.socket->getShort();
		const auto json = connection.socket->getStringByLength(jsonLength);

		APG::JSONSerializer<AuthenticationIdentity> idDecoder;
		const AuthenticationIdentity authID = idDecoder.fromJSON(json.c_str());

		processLoginAttempt(connection, authID, logger);
	} else if (opcode == static_cast<opcode_type_t>(ClientOpcode::VERSION_MISMATCH)) {
		// we can't really help in this case
		logger->info("Client had mismatched version.");
		connection.state = IncomingConnectionState::DONE;
		return;
	} else {
		logger->info("Client sent unexpected data.");
		// unexpected input; increase their attempts
		connection.state = IncomingConnectionState::LOGIN_FAILED;
		connection.loginAttempts += 1;

		AuthenticationResponse response(false, connection.getAttemptsRemaining(), "Unexpected data.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();
	}

	if (connection.loginAttempts == IncomingConnection::MAX_ATTEMPTS_ALLOWED) {
		// exhausted their attempts
		connection.state = IncomingConnectionState::DONE;
	}
}

void LoginServer::processLoginFailedSocket(IncomingConnection &connection, el::Logger * const logger) {
	// Similar to CHALLENGE_SENT state, except we can ignore VersionMismatch packets
	if (!connection.socket->hasActivity()) {
		return;
	}

	logger->verbose(9, "Handling LOGIN_FAILED connection.");

	int dataRec = connection.socket->recv(4096);

	logger->info("Read %v bytes", dataRec);

	if (dataRec <= 0) {
		logger->error("No data received in LOGIN_FAILED, dropping connection.");
		connection.state = IncomingConnectionState::DONE;
		return;
	}

	const auto opcode = connection.socket->getShort();

	if (opcode == static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY)) {
		// attempt auth
		const auto jsonLength = connection.socket->getShort();
		const auto json = connection.socket->getStringByLength(jsonLength);

		logger->verbose(9, "Client attempted auth in LOGIN_FAILED, json: %v", json);

		APG::JSONSerializer<AuthenticationIdentity> idDecoder;
		const AuthenticationIdentity authID = idDecoder.fromJSON(json.c_str());

		processLoginAttempt(connection, authID, logger);
	} else {
		logger->info("Client sent unexpected data in LOGIN_FAILED, opcode %v, bytesRec %v", opcode, dataRec);
		// unexpected input; increase their attempts
		connection.state = IncomingConnectionState::LOGIN_FAILED;
		connection.loginAttempts += 1;

		AuthenticationResponse response(false, connection.getAttemptsRemaining(), "Unexpected data.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();
	}

	if (connection.loginAttempts == IncomingConnection::MAX_ATTEMPTS_ALLOWED) {
		// exhausted their attempts
		connection.state = IncomingConnectionState::DONE;
	}
}

void LoginServer::processDoneSocket(IncomingConnection &connection, el::Logger * const logger) {
	// NO-OP
}

bool LoginServer::processLoginAttempt(IncomingConnection &connection, const AuthenticationIdentity &authID,
        el::Logger * const logger) {
//	auto ps = std::unique_ptr<sql::Statement>(mysqlConnection->createStatement());
//
//	auto res = std::unique_ptr<sql::ResultSet>(ps->executeQuery("SELECT * FROM players;"));
//
//	while (res->next()) {
//		std::cout << "id = " << res->getInt("id") << "\nname = " << res->getString("email") << std::endl;
//	}

	std::string passStr((const char*)authID.password.data(), authID.password.size());

	logger->info("Password received (len = %v)", authID.password.size());
	logger->info("Password decrypted: %v", crypto.decryptStringPrivate(passStr));

	if (authID.username == "SgtCoDFish@example.com") {
		// success

		AuthenticationResponse response(true, connection.getAttemptsRemaining(), "Authentication successful.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();

		auto newSession = std::make_unique<PlayerSession>(authID.username, std::move(connection.socket));

		logger->verbose(9, "New user session for \"%v\": GUID %v.", newSession->username, newSession->guid);

		playerSessions.emplace_back(std::move(newSession));

		connection.state = IncomingConnectionState::DONE;

		return true;
	} else {
		connection.state = IncomingConnectionState::LOGIN_FAILED;
		connection.loginAttempts += 1;

		logger->verbose(9, "Login failed.");

		AuthenticationResponse response(false, connection.getAttemptsRemaining(), "Authentication failure.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();
		connection.socket->clear();

		return false;
	}
}

}
