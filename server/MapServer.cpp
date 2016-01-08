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

#include <cstring>

#include <utility>

#include <APG/APGS11N.hpp>
#include <APG/core/APGeasylogging.hpp>

#include <tmxparser/Tmx.h>

#include "net/packets/LoginPackets.hpp"
#include "MapServer.hpp"
#include "PlayPGVersion.hpp"

namespace PlayPG {

MapServer::MapServer(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_,
        std::vector<std::string> &&maps, const std::string &masterServer_, const uint16_t &masterPort_,
        const std::string &masterPublicKeyFile_, const std::string &masterPrivateKeyFile_) :
		        Server(serverDetails_, databaseDetails_),
		        mapPaths { std::move(maps) },
		        masterServerHostname { masterServer_ },
		        masterServerPort { masterPort_ },
		        masterServerCrypto { RSACrypto::fromFiles(masterPublicKeyFile_, masterPrivateKeyFile_) },
		        masterServerConnection { getSocket(masterServerHostname, masterServerPort) } {
}

void MapServer::run() {
	auto logger = el::Loggers::getLogger("ServPG");

	logger->info("Running map server.");

	if (!parseMaps(logger)) {
		return;
	}

	if (!registerWithMasterServer(logger)) {
		return;
	}
}

bool MapServer::registerWithMasterServer(el::Logger * const logger) {
	logger->info("Establishing connection with \"%v\" on port %v.", masterServerHostname, masterServerPort);

	masterServerConnection->connect();

	if (masterServerConnection->hasError()) {
		logger->error("Couldn't connect to master login server.");
		return false;
	}

	logger->verbose(5, "Waiting for master server to send authentication challenge.");
	masterServerConnection->waitForActivity(5000);

	masterServerConnection->recv();

	// we expect to get an authentication challenge from the master server

	const auto opcode = masterServerConnection->getShort();

	if (opcode != static_cast<opcode_type_t>(ServerOpcode::LOGIN_AUTHENTICATION_CHALLENGE)) {
		logger->error("Got opcode %v which doesn't match authentication challenge; bad master login server.");
		return false;
	}

	const uint16_t jsonSize = masterServerConnection->getShort();

	auto json = masterServerConnection->getStringByLength(jsonSize);

	APG::JSONSerializer<AuthenticationChallenge> challengeS11N;
	const auto challenge = challengeS11N.fromJSON(json.c_str());

	logger->info("Got challenge from \"%v\", v%v (%v).", challenge.name, challenge.version, challenge.versionHash);
	masterServerConnection->clear();

	// Check the server's public key matches the one we've loaded in; if it doesn't we can't continue.
	if (challenge.pubKey != masterServerCrypto->getPublicKeyPEM()) {
		logger->error("Server's public key doesn't match the one loaded; incorrect key file.");
		return false;
	} else {
		logger->verbose(9, "Server's public key matches expected key.");
	}

	if (std::strcmp(challenge.version.c_str(), Version::versionString) != 0
	        || std::strcmp(challenge.versionHash.c_str(), Version::gitHash) != 0) {
		logger->info("Version check failed.");

		VersionMismatch mismatchPacket;

		masterServerConnection->put(&mismatchPacket.buffer);
		masterServerConnection->send();

		return false;
	}

	logger->info("Version check successful.");

	MapServerRegistrationRequest regRequest(serverDetails.friendlyName);

	masterServerConnection->put(&regRequest.buffer);
	masterServerConnection->send();
	masterServerConnection->clear();

	if (!masterServerConnection->waitForActivity(3000)) {
		logger->error("Master server timed out while waiting for response to registration request.");

		return false;
	}

	auto regRequestResponseBytes = masterServerConnection->recv();

	if (regRequestResponseBytes <= 0) {
		logger->error("Error while receiving response to registration request.");

		return false;
	}

	const auto regRequestOpcode = masterServerConnection->getShort();

	if (regRequestOpcode != util::to_integral(ServerOpcode::MAP_SERVER_REGISTRATION_RESPONSE)) {
		logger->error("Bad master server: incorrect registration response sent: %v", opcode);

		return false;
	}

	auto mapHashes = MapServerMapList::listFromMaps(maps);

	masterServerConnection->clear();
	masterServerConnection->put(&mapHashes.buffer);
	masterServerConnection->send();

	logger->verbose(9, "Sent map list with %v maps", mapHashes.mapHashes.size());

	if (!masterServerConnection->waitForActivity(3000)) {
		logger->error("Error receiving response to map list.");

		return false;
	}

	return true;
}

bool MapServer::parseMaps(el::Logger * const logger) {
	for (const auto & mapPath : mapPaths) {
		auto map = std::make_unique<Tmx::Map>();

		map->ParseFile(mapPath);

		if (map->HasError()) {
			logger->error("Couldn't parse %v: %v.", mapPath, map->GetErrorText());
			continue;
		}

		tmxparserMaps.emplace_back(std::move(map));
	}

	if (tmxparserMaps.size() < mapPaths.size()) {
		// some maps failed to load
		logger->error("Couldn't load %v maps.", (mapPaths.size() - tmxparserMaps.size()));

		if (tmxparserMaps.size() == 0) {
			return false;
		}
	} else {
		logger->verbose(1, "Parsed all tmxparser maps successfully.");
	}

	for (const auto &map : tmxparserMaps) {
		maps.emplace_back(Map(map.get()));
	}

	return true;
}

}
