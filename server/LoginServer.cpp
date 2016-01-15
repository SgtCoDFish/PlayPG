/*
 * Copyright (c) 2015,2016 See AUTHORS file.
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
#include <APG/internal/Assert.hpp>

#include <odb/transaction.hxx>

#include "net/Opcodes.hpp"
#include "net/packets/LoginPackets.hpp"
#include "PlayPGVersion.hpp"

#include "Player.hpp"
#include "odb/Player_odb.hpp"

#include "Location.hpp"
#include "odb/Location_odb.hpp"

namespace PlayPG {

LoginServer::LoginServer(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_,
        bool makeNewKeys_) :
		        Server(serverDetails_, databaseDetails_),
		        regenerateKeys_ { makeNewKeys_ } {
	playerAcceptor = getAcceptorSocket(serverDetails.port, true);

	if (serverDetails.maps == boost::none) {
		el::Loggers::getLogger("ServPG")->fatal("No maps passed to LoginServer through ServerDetails");
		return;
	}

	if (regenerateKeys_) {
		el::Loggers::getLogger("ServPG")->info("Generating RSA public/private key pair.");
		crypto = std::make_unique<RSACrypto>(true);

		const std::string pubKeyFile = (serverDetails.publicKeyFile ? *serverDetails.publicKeyFile : "login.pub");
		const std::string priKeyFile = (serverDetails.privateKeyFile ? *serverDetails.privateKeyFile : "login.prv");

		crypto->writePublicKeyFile(pubKeyFile);
		crypto->writePrivateKeyFile(priKeyFile);
		el::Loggers::getLogger("ServPG")->info("Dumped keys to %v (public) and %v (private).", pubKeyFile, priKeyFile);
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

	processMaps(logger);

	logger->info("Running login server on port %v.", serverDetails.port);
	logger->info("\"%v\", version %v (%v)", serverDetails.friendlyName, Version::versionString, Version::gitHash);

	processingThread = std::thread([this]() {this->processIncoming();});
	connectedThread = std::thread([this]() {this->processConnected();});

	initDB(logger);

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
	connectedThread.join();
}

void LoginServer::processMaps(el::Logger * const logger) {
	auto &mapPaths = serverDetails.maps.get();

	for (const auto & mapPath : mapPaths) {
		auto map = std::make_unique<Tmx::Map>();

		map->ParseFile(mapPath);

		if (map->HasError()) {
			logger->error("Couldn't parse %v: %v.", mapPath, map->GetErrorText());
			continue;
		}

		auto loadedLocation = Location(Map::resolveNameFromMap(map.get(), logger), mapPath,
		        Location::makeMD5Hash(map->GetFilehash()), Map::resolveVersionFromMap(map.get(), logger));

		odb::transaction t(db->begin());

		odb::query<Location> q(odb::query<Location>::locationName == odb::query<Location>::_ref(loadedLocation.locationName));

		auto results = db->query<Location>(q);

#ifndef NDEBUG
		if (results.size() > 1) {
			logger->warn(
			        "More than one location was found for the map named \"%v\". This is a possible data integrity issue.",
			        loadedLocation.locationName);
		}
#endif

		if (results.empty()) {
			// New map
			auto newLocID = db->persist(loadedLocation);

			logger->info("Created new db entry for map %v (id %v)", loadedLocation.locationName, newLocID);
		} else {
			// Existing map, need to check its integrity
			auto locIterator = results.begin();
			Location databaseLocation = *locIterator;

			if (loadedLocation.version == databaseLocation.version) {
				if (locIterator->knownMD5Hash != loadedLocation.knownMD5Hash) {
					/*
					 * As implemented, this is incredibly unlikely because changing the version number will change the hash.
					 *
					 * This probably should be changed.
					 */
					logger->error("Map hash difference for %v (id %v), despite matching version number: %v.",
					        databaseLocation.locationName, databaseLocation.id, loadedLocation.version);
					logger->error("This is almost certainly a mistake with versioning.");
				} else {
					logger->verbose(8, "Map %v matches expected values.", loadedLocation.locationName);
				}
			} else {
				if (loadedLocation.version > databaseLocation.version) {
					// We have a newer version of the map; update in the database.

					databaseLocation.knownMD5Hash = loadedLocation.knownMD5Hash;
					databaseLocation.version = loadedLocation.version;

					db->update<Location>(databaseLocation);
					logger->verbose(1, "Updated map %v to new version %v in the database.", loadedLocation.locationName,
							loadedLocation.version);
				} else {
					logger->error(
					        "Map %v is outdated according to the database. Try replacing it with the newer version \"%v\".",
					        loadedLocation.locationName, databaseLocation.version);
					/*
					 * There's nothing sensible to really do here;
					 * we'll keep trying to start but likely the server won't be much use
					 * with an outdated map.
					 */
				}
			}
		}

		t.commit();
		allMaps.emplace_back(std::move(loadedLocation));
	}

	mapServers.reserve(allMaps.size());

}

void LoginServer::initDB(el::Logger * const logger) {
	odb::transaction t(db->begin());

	PlayerCount playerCount(db->query_value<PlayerCount>());

	if (playerCount.count == 0u) {
		static const char * const suID = "SgtCoDFish@example.com";
		static const char * const suPWD = "testa";

		logger->info("DB is empty, creating super-user \"%v\".", suID);

		const auto salt = hasher.generateSalt();
		const auto hashedPassword = hasher.hashPasswordSHA512(suPWD, salt);

		const auto pwdString = hasher.sha512ToString(hashedPassword);
		const auto saltString = hasher.saltToString(salt);

		Player superUser(suID, pwdString, saltString);
		superUser.id = 1;
		superUser.joinDate = superUser.lastLogin = boost::posix_time::second_clock::universal_time();

		db->persist(superUser);

		t.commit();

		logger->info("Created super-user.");
	}
}

}
