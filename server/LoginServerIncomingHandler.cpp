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

#include <APG/internal/Assert.hpp>

#include <odb/transaction.hxx>
#include <odb/result.hxx>

#include "LoginServer.hpp"

#include "PlayPGVersion.hpp"
#include "Player.hpp"
#include "odb/Player_odb.hpp"

namespace PlayPG {

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

			case (IncomingConnectionState::MAP_LIST): {
				processMapListSocket(connection, logger);
				break;
			}

			case (IncomingConnectionState::MAP_WAIT_ACK): {
				processMapWaitAckSocket(connection, logger);
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
				logger->verbose(5, "Purged %v incoming sockets.", removed);
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

	connection.socket->clear();
	connection.socket->put(&challenge.buffer);

	connection.socket->send();

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
	 * - MapServerRegistrationRequest: if they fail, we disconnect them immediately (a real map server has
	 * 								   our private key and they should be right first time, every time
	 * - Something else: they lose a login attempt and go again/get disconnected
	 */

	if (!connection.socket->hasActivity()) {
		return;
	}

	logger->verbose(9, "Handling CHALLENGE_SENT.");

	const auto bytesFromChallenge = connection.socket->recv(2048);

//	logger->info("Read %v bytes", bytesFromChallenge);

	if (bytesFromChallenge == 0) {
		logger->error("Couldn't read after CHALLENGE_SENT");
		connection.state = IncomingConnectionState::DONE;
		return;
	}

	const auto opcode = connection.socket->getShort();

	if (opcode == static_cast<opcode_type_t>(ClientOpcode::LOGIN_AUTHENTICATION_IDENTITY)) {
		// attempt auth
		const auto jsonLength = connection.socket->getShort();
		const auto json = connection.socket->getStringByLength(jsonLength);
		connection.socket->clear();

		APG::JSONSerializer<AuthenticationIdentity> idDecoder;
		const AuthenticationIdentity authID = idDecoder.fromJSON(json.c_str());

		processLoginAttempt(connection, authID, logger);
	} else if (opcode == static_cast<opcode_type_t>(ClientOpcode::VERSION_MISMATCH)) {
		// we can't really help in this case
		logger->verbose(9, "Client had mismatched version.");
		connection.socket->clear();

		connection.state = IncomingConnectionState::DONE;
		return;
	} else if (opcode == static_cast<opcode_type_t>(ServerOpcode::MAP_SERVER_REGISTRATION_REQUEST)) {
		const auto nameLen = connection.socket->getShort();
		const auto name = connection.socket->getStringByLength(nameLen);

		const auto addressLen = connection.socket->getShort();
		const auto address = connection.socket->getStringByLength(addressLen);

		const auto port = connection.socket->getShort();

		connection.socket->clear();

		connection.mapServerFriendlyName = name;
		connection.mapServerListenAddress = address;
		connection.mapServerPort = port;

		logger->verbose(9, "Got a map server registration request from %v (%v on port %v)", name, address, port);

		if (!processMapAuthenticationRequest(connection, logger)) {
			return;
		}
	} else {
		logger->verbose(9, "Client sent unexpected data (opcode %v)", opcode);
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

	const int dataRec = connection.socket->recv(2048);

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

void LoginServer::processMapListSocket(IncomingConnection &connection, el::Logger * const logger) {
	if (!connection.socket->hasActivity()) {
		return;
	}

	logger->verbose(9, "Handling MAP_LIST connection.");

	const auto listBytes = connection.socket->recv();

	if (listBytes == 0) {
		logger->info("Map server failed to correctly send a map list; dropping.");
		connection.state = IncomingConnectionState::DONE;
		return;
	}

	const auto opcode = connection.socket->getShort();

	logger->info("Got opcode: %v", opcode);

	if (opcode != static_cast<opcode_type_t>(ServerOpcode::MAP_SERVER_MAP_LIST)) {
		logger->info("Map server didn't send map list; dropping.");
		connection.state = IncomingConnectionState::DONE;
		return;
	}

	const auto jsonLength = connection.socket->getShort();

	APG::JSONSerializer<MapServerMapList> jsonSerializer;

	const auto json = connection.socket->getStringByLength(jsonLength);

	logger->info("Got json: %v", json);

	const auto mapList = jsonSerializer.fromJSON(json.c_str());

	std::vector<MapIdentifier> responseMapList;

	for (const auto &mapID : mapList.mapHashes) {
		bool weSupport = false;
		bool weIgnore = true;

		for (const auto &ourMap : allMaps) {
			if (mapID.mapName == ourMap.mapName) {
				if (mapID.mapHash == ourMap.mapHash) {
					weSupport = true;

					if (mapNameToConnection.find(mapID.mapName) == mapNameToConnection.end()) {
						// already have a server supporting this map.
						responseMapList.emplace_back(MapIdentifier(mapID));
						weIgnore = false;
					} else {
						weIgnore = true;
					}

				} else {
					logger->warn("Map names same but hash differs for %v.", mapID.mapName);
				}

				break;
			}
		}

		logger->verbose(7, "Map server supports \"%v\" with hash: %v (%v by this login server) (%v by this server)",
		        mapID.mapName, mapID.mapHash, (weSupport ? "recognised" : "not recognised"),
		        (weIgnore ? "ignored" : "not ignored"));
	}

	if (responseMapList.empty()) {
		logger->verbose(1, "Map server doesn't support any new maps for this login server, ignoring.");

		connection.socket->clear();
		connection.socket->putShort(util::to_integral(ServerOpcode::MAP_SERVER_NOT_NEEDED));
		connection.socket->send();

		connection.state = IncomingConnectionState::DONE;
		return;
	}

	MapServerMapList requiredMaps(responseMapList);

	connection.socket->clear();
	connection.socket->put(&requiredMaps.buffer);
	const auto reqMapBytes = connection.socket->send();

	if (reqMapBytes == 0) {
		logger->error("Couldn't send required maps to map server.");
	}

	connection.maps = std::move(responseMapList);

	connection.state = IncomingConnectionState::MAP_WAIT_ACK;
}

void LoginServer::processMapWaitAckSocket(IncomingConnection &connection, el::Logger * const logger) {
	if (!connection.socket->hasActivity()) {
		return;
	}

	REQUIRE(connection.maps != boost::none, "Connection maps must be initialised when waiting for ack.");
	REQUIRE(connection.mapServerFriendlyName != boost::none,
	        "Connection friendly name must be initialised when waiting for ack.");
	REQUIRE(connection.mapServerPort != boost::none, "Connection port must be initialised when waiting for ack.");
	REQUIRE(connection.mapServerListenAddress != boost::none,
	        "Connection address must be initialised when waiting for ack.");

	const auto bytesRec = connection.socket->recv();

	if (bytesRec == 0) {
		logger->error("Couldn't receive map ACK");
		connection.state = IncomingConnectionState::DONE;

		return;
	}

	{
		// lock guard avoids concurrent modification + ensures .back() is valid while we're using it
		std::lock_guard<std::mutex> mapGuard(mapServersMutex);

		mapServers.emplace_back(*connection.mapServerListenAddress, *connection.mapServerPort,
		        std::move(connection.socket), std::move(*(connection.maps)), *connection.mapServerFriendlyName);

		const auto &newestServer = mapServers.back();

		logger->info("Registered new map server \"%v\": %v on port %v.", newestServer.friendlyName,
		        newestServer.hostname, newestServer.port);

		for (const auto &map : newestServer.maps) {
			mapNameToConnection.emplace(
			        std::pair<std::string, const MapServerConnection *>(map.mapName, &newestServer));
		}

	}

	connection.state = IncomingConnectionState::DONE;
}

void LoginServer::processDoneSocket(IncomingConnection &connection, el::Logger * const logger) {
// NO OP
}

bool LoginServer::processLoginAttempt(IncomingConnection &connection, const AuthenticationIdentity &authID,
        el::Logger * const logger) {
	const auto decPass = crypto->decryptStringPrivate(authID.password);

	odb::transaction t(db->begin());

	odb::query<Player> q(odb::query<Player>::_ref(authID.username) == odb::query<Player>::username);

	odb::result<Player> result(db->query<Player>(q));

	if (result.empty()) {
		connection.state = IncomingConnectionState::LOGIN_FAILED;
		connection.loginAttempts += 1;

		logger->verbose(9, "Login failed; invalid username.");

		AuthenticationResponse response(false, connection.getAttemptsRemaining(), "Authentication failure.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();

		t.commit();
		return false;
	}

#ifndef NDEBUG
	if (result.size() > 1) {
		logger->warn("Possible data integrity issue; multiple rows retrieved for email %v. Using first only.",
		        authID.username);
	}
#endif

	const auto person = result.begin();

	const uint32_t playerID = person->id;
	const std::string sha512 = person->password;
	const std::string saltString = person->salt;
	const auto dbSalt = hasher.stringToSalt(saltString);

	const auto hashedPass = hasher.hashPasswordSHA512(decPass, dbSalt);

	const std::string hashString = hasher.sha512ToString(hashedPass);

	if (sha512 == hashString) {
		AuthenticationResponse response(true, connection.getAttemptsRemaining(), "Authentication successful.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();

		auto newSession = std::make_unique<PlayerSession>(playerID, authID.username, std::move(connection.socket),
		        random);

		logger->verbose(9, "New user session for \"%v\": GUID %v.", newSession->username, newSession->guid);

		{
			// ensure .back() stays valid
			std::lock_guard<std::mutex> playerSessionGuard(playerSessionMutex);
			playerSessions.emplace_back(std::move(newSession));

			const auto &back = playerSessions.back();

			usernameToPlayerSession.emplace(std::pair<std::string, const PlayerSession *>(back->username, back.get()));
		}

		connection.state = IncomingConnectionState::DONE;

		t.commit();
		return true;
	} else {
		t.commit();
		connection.state = IncomingConnectionState::LOGIN_FAILED;
		connection.loginAttempts += 1;

		logger->verbose(9, "Login failed; incorrect password.");

		AuthenticationResponse response(false, connection.getAttemptsRemaining(), "Authentication failure.");

		connection.socket->clear();
		connection.socket->put(&response.buffer);
		connection.socket->send();

		return false;
	}
}

bool LoginServer::processMapAuthenticationRequest(IncomingConnection &connection, el::Logger * const logger) {
	MapServerRegistrationResponse regResponse("hello, world");

	connection.socket->clear();
	connection.socket->put(&regResponse.buffer);

	const int respSent = connection.socket->send();

	if (respSent <= 0) {
		logger->error("Couldn't send map server registration response.");
		return false;
	}

	logger->verbose(9, "Sent %v map registration response bytes.", respSent);

	connection.state = IncomingConnectionState::MAP_LIST;

	return true;
}

}
