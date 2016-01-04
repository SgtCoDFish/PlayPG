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
#include <cppconn/prepared_statement.h>

#include "net/Opcodes.hpp"
#include "net/packets/LoginPackets.hpp"
#include "PlayPGVersion.hpp"

namespace PlayPG {

LoginServer::LoginServer(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_,
        bool makeNewKeys_) :
		        Server(serverDetails_, databaseDetails_),
		        regenerateKeys_ { makeNewKeys_ } {
	playerAcceptor = getAcceptorSocket(serverDetails_.port, true);

	if (regenerateKeys_) {
		el::Loggers::getLogger("ServPG")->info("Generating RSA public/private key pair.");
		crypto = std::make_unique<RSACrypto>(true);
	} else {
		const std::string pubKeyFile = (serverDetails.publicKeyFile ? *serverDetails.publicKeyFile : "login.pub");
		const std::string priKeyFile = (serverDetails.privateKeyFile ? *serverDetails.privateKeyFile : "login.prv");

		el::Loggers::getLogger("ServPG")->info("Loading RSA public/private key pair from %v (public) and %v (private).",
		        pubKeyFile, priKeyFile);

		crypto = RSACrypto::fromFiles(pubKeyFile, priKeyFile);

		if (crypto == nullptr) {
			el::Loggers::getLogger("ServPG")->fatal("Couldn't load keypair from files %v (public) and %v (private).",
			        pubKeyFile, priKeyFile);
		}
	}
}

void LoginServer::run() {
	auto logger = el::Loggers::getLogger("ServPG");

	logger->info("Running login server on port %v.", serverDetails.port);
	logger->info("\"%v\", version %v (%v)", serverDetails.friendlyName, Version::versionString, Version::gitHash);

	processingThread = std::thread([this]() {this->processIncoming();});

	initDB(logger);

	if (regenerateKeys_) {
		const std::string pubKeyFile = (serverDetails.publicKeyFile ? *serverDetails.publicKeyFile : "login.pub");
		const std::string priKeyFile = (serverDetails.privateKeyFile ? *serverDetails.privateKeyFile : "login.prv");

		crypto->writePublicKeyFile(pubKeyFile);
		crypto->writePrivateKeyFile(priKeyFile);
		logger->info("Dumped keys to %v (public) and %v (private).", pubKeyFile, priKeyFile);
	}

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
	AuthenticationChallenge challenge(Version::versionString, Version::gitHash, serverDetails.friendlyName,
	        crypto->getPublicKeyPEM());

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
		logger->verbose(9, "Client had mismatched version.");
		connection.state = IncomingConnectionState::DONE;
		return;
	} else if (opcode == static_cast<opcode_type_t>(ServerOpcode::MAP_SERVER_REGISTRATION_REQUEST)) {
		const auto strLen = connection.socket->getShort();
		const auto str = connection.socket->getStringByLength(strLen);

		logger->verbose(9, "Got a map server registration request from %v", str);

		if (!processMapAuthenticationRequest(connection, logger)) {
			return;
		}
	} else {
		logger->verbose(9, "Client sent unexpected data.");
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

	const int dataRec = connection.socket->recv(4096);

	logger->info("Read %v bytes", dataRec);

	if (dataRec <= 0) {
		logger->error("No data received in LOGIN_FAILED, dropping connection.");
		connection.state = IncomingConnectionState::DONE;
		return;
	}

	const auto opcode = connection.socket->getShort();

	// Note that we ignore map server auth requests here. They only get one shot.

	if (opcode == static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY)) {
		// attempt auth
		const auto jsonLength = connection.socket->getShort();
		const auto json = connection.socket->getStringByLength(jsonLength);

		logger->verbose(9, "Client attempted auth in LOGIN_FAILED");

		APG::JSONSerializer<AuthenticationIdentity> idDecoder;
		const AuthenticationIdentity authID = idDecoder.fromJSON(json.c_str());

		processLoginAttempt(connection, authID, logger);
	} else {
		logger->info("Client sent unexpected data in LOGIN_FAILED", opcode, dataRec);
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
// NO OP
}

bool LoginServer::processLoginAttempt(IncomingConnection &connection, const AuthenticationIdentity &authID,
        el::Logger * const logger) {
	const auto decPass = crypto->decryptStringPrivate(authID.password);

	auto statement = std::unique_ptr<sql::PreparedStatement>(
	        mysqlConnection->prepareStatement("SELECT * FROM players WHERE email=?;"));
	statement->setString(1, authID.username);

	auto results = std::unique_ptr<sql::ResultSet>(statement->executeQuery());

	const auto rowCount = results->rowsCount();

	if (rowCount == 0) {
		connection.state = IncomingConnectionState::LOGIN_FAILED;
		connection.loginAttempts += 1;

		logger->verbose(9, "Login failed; invalid username.");

		AuthenticationResponse response(false, connection.getAttemptsRemaining(), "Authentication failure.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();
		connection.socket->clear();

		return false;
	}

	if (rowCount > 1) {
		logger->warn("Possible data integrity issue; multiple rows retrieved for email %v. Using first only.",
		        authID.username);
	}

	results->next();

	const std::string sha512 = results->getString("password");
	const std::string saltString = results->getString("salt");
	const auto dbSalt = hasher.stringToSalt(saltString);

	const auto hashedPass = hasher.hashPasswordSHA512(decPass, dbSalt);

	const std::string hashString = hasher.sha512ToString(hashedPass);

	if (sha512 == hashString) {
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

		logger->verbose(9, "Login failed; incorrect password.");

		AuthenticationResponse response(false, connection.getAttemptsRemaining(), "Authentication failure.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();
		connection.socket->clear();

		return false;
	}
}

bool LoginServer::processMapAuthenticationRequest(IncomingConnection &connection, el::Logger * const logger) {
	MapServerRegistrationResponse regResponse("hello, world");

	connection.socket->clear();
	connection.socket->put(&regResponse.buffer);
	connection.socket->send();

	connection.state = IncomingConnectionState::DONE;

	return true;
}

void LoginServer::initDB(el::Logger * const logger) {
	auto ps = std::unique_ptr<sql::Statement>(mysqlConnection->createStatement());

	auto res = std::unique_ptr<sql::ResultSet>(ps->executeQuery("SELECT COUNT(*) AS playerCount FROM players;"));

	res->next();
	const auto playerCount = res->getUInt("playerCount");

	if (playerCount == 0u) {
		static const char * const suID = "SgtCoDFish@example.com";
		static const char * const suPWD = "testa";

		const auto salt = hasher.generateSalt();
		const auto hashedPassword = hasher.hashPasswordSHA512(suPWD, salt);

		logger->info("DB is empty, creating super-user \"%v\".", suID);

		auto insert = std::unique_ptr<sql::Statement>(mysqlConnection->createStatement());

		const auto pwdString = hasher.sha512ToString(hashedPassword);
		const auto saltString = hasher.saltToString(salt);

//		logger->info("\nPassword: %v\n"
//				"Salt    : %v", pwdString, saltString);

		const char * const sqlString = "INSERT INTO players (email, password, salt) VALUES (?, ?, ?);";

		auto prep = std::unique_ptr<sql::PreparedStatement>(mysqlConnection->prepareStatement(sqlString));
		prep->setString(1, suID);
		prep->setString(2, pwdString);
		prep->setString(3, saltString);

		prep->execute();

		logger->info("Created super-user.");
	}
}

}
