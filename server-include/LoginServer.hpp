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

#ifndef INCLUDE_SERVER_LOGINSERVER_HPP_
#define INCLUDE_SERVER_LOGINSERVER_HPP_

#include <list>
#include <vector>
#include <memory>
#include <array>
#include <thread>
#include <utility>
#include <mutex>

#include <boost/optional.hpp>

#include "ServerCommon.hpp"
#include "net/PlayerSession.hpp"
#include "net/Opcodes.hpp"
#include "net/packets/LoginPackets.hpp"

#include "net/crypto/RSACrypto.hpp"
#include "net/crypto/SHACrypto.hpp"

#include "Map.hpp"

#include <APG/core/APGeasylogging.hpp>

namespace PlayPG {

enum class IncomingConnectionState {
	FRESH, // Just connected, needs an auth challenge sending
	CHALLENGE_SENT, // Challenge has been sent, waiting for response.
	LOGIN_FAILED, // Login failed for some reason and the user has been given the
	              // chance to try again.
	MAP_LIST, // The login server is waiting for the map server to send its list of supported maps.
	MAP_WAIT_ACK, // The login server is waiting for the map server to acknowledge that it will
	              // support the maps the login server requested.
	DONE // Login was successful/some error caused the socket to close.
	     // Won't be in this state for long; after this the
	     // user should be sent to a map server to actually play.
};

struct IncomingConnection {
	static constexpr const int MAX_ATTEMPTS_ALLOWED = 3;

	explicit IncomingConnection(std::unique_ptr<APG::Socket> &&socket_) :
			        socket { std::move(socket_) },
			        state { IncomingConnectionState::FRESH } {
	}
	~IncomingConnection() = default;

	IncomingConnection &operator=(IncomingConnection&& ic) = default;
	IncomingConnection(IncomingConnection&& ic) = default;

	std::unique_ptr<APG::Socket> socket;

	IncomingConnectionState state;

	int loginAttempts = 0;

	int getAttemptsRemaining() const {
		return MAX_ATTEMPTS_ALLOWED - loginAttempts;
	}

	boost::optional<std::string> mapServerFriendlyName = boost::none;
	boost::optional<uint16_t> mapServerPort = boost::none;
	boost::optional<std::string> mapServerListenAddress = boost::none;
	boost::optional<std::vector<MapIdentifier>> maps = boost::none;
};

struct MapServerConnection {
	explicit MapServerConnection(const std::string &hostname_, const uint16_t &port_,
	        std::unique_ptr<APG::Socket> &&connection_, std::vector<MapIdentifier> &&maps_,
	        const std::string &friendlyName_) :
			        connection { std::move(connection_) },
			        maps { std::move(maps_) },
			        friendlyName { friendlyName_ },
			        hostname { hostname_ },
			        port { port_ } {
	}

	std::unique_ptr<APG::Socket> connection;
	const std::vector<MapIdentifier> maps;

	const std::string friendlyName;

	const std::string hostname;
	const uint16_t port;
};

class LoginServer final : public Server {
public:
	explicit LoginServer(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_,
	        bool regenerateKeys_ = false);
	virtual ~LoginServer() = default;

	virtual void run() override final;

	/**
	 * Probably run in a separate thread.
	 */
	void processIncoming();

	/**
	 * Probably run in a separate thread; manages players who're already connected.
	 */
	void processConnected();

private:
	void initDB(el::Logger * const logger);
	void processMaps(el::Logger * const logger);

	// Methods used to process incoming connections
	void processFreshSocket(IncomingConnection &connection, AuthenticationChallenge &challange,
	        el::Logger * const logger);
	void processChallengeSentSocket(IncomingConnection &connection, el::Logger * const logger);
	void processLoginFailedSocket(IncomingConnection &connection, el::Logger * const logger);
	void processMapListSocket(IncomingConnection &connection, el::Logger * const logger);
	void processMapWaitAckSocket(IncomingConnection &connection, el::Logger * const logger);
	void processDoneSocket(IncomingConnection &connection, el::Logger * const logger);
	bool processLoginAttempt(IncomingConnection &connection, const AuthenticationIdentity &id,
	        el::Logger * const logger);
	bool processMapAuthenticationRequest(IncomingConnection &connection, el::Logger * const logger);

	// Methods used to process connections which have already been established.
	void processCharacterRequest(const std::unique_ptr<PlayerSession> &session, el::Logger * const logger);
	void processCharacterSelect(const std::unique_ptr<PlayerSession> &session, el::Logger * const logger);

	bool regenerateKeys_ = false;
	std::unique_ptr<RSACrypto> crypto;
	SHACrypto hasher { 32000 };

	// For accepting connections from players
	std::unique_ptr<APG::AcceptorSocket> playerAcceptor;

	std::list<std::unique_ptr<PlayerSession>> playerSessions;
	std::unordered_map<std::string, const PlayerSession *> usernameToPlayerSession;
	std::mutex playerSessionMutex;

	// connections which haven't authenticated themselves yet and so cannot have a session made.
	std::vector<IncomingConnection> incomingConnections;
	std::mutex incomingConnectionsMutex;

	std::vector<MapIdentifier> allMaps;

	// A list of connected map servers
	std::vector<MapServerConnection> mapServers;
	std::unordered_map<std::string, const MapServerConnection *> mapNameToConnection;

	std::mutex mapServersMutex;

	bool done = false;
	std::thread processingThread;
	std::thread connectedThread;
};

}

#endif /* INCLUDE_SERVER_LOGINSERVER_HPP_ */
