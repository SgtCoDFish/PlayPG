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

#include <vector>
#include <memory>
#include <array>
#include <thread>
#include <utility>
#include <mutex>

#include "ServerCommon.hpp"
#include "net/PlayerSession.hpp"
#include "net/Opcodes.hpp"
#include "net/packets/LoginPackets.hpp"

#include "net/crypto/RSACrypto.hpp"
#include "net/crypto/SHACrypto.hpp"

#include <APG/core/APGeasylogging.hpp>

namespace PlayPG {

enum class IncomingConnectionState {
	FRESH, // Just connected, needs an auth challenge sending
	CHALLENGE_SENT, // Challenge has been sent, waiting for response.
	LOGIN_FAILED, // Login failed for some reason and the user has been given the
	              // chance to try again.
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
};

class LoginServer final : public Server {
public:
	explicit LoginServer(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_, bool regenerateKeys_ = false);
	virtual ~LoginServer() = default;

	virtual void run() override final;

	/**
	 * Probably run in a separate thread.
	 */
	void processIncoming();

private:
	void initDB(el::Logger * const logger);

	void processFreshSocket(IncomingConnection &connection, AuthenticationChallenge &challange,
	        el::Logger * const logger);
	void processChallengeSentSocket(IncomingConnection &connection, el::Logger * const logger);
	void processLoginFailedSocket(IncomingConnection &connection, el::Logger * const logger);
	void processDoneSocket(IncomingConnection &connection, el::Logger * const logger);
	bool processLoginAttempt(IncomingConnection &connection, const AuthenticationIdentity &id,
	        el::Logger * const logger);

	bool processMapAutenticationRequest(IncomingConnection &connection, el::Logger * const logger);

	bool regenerateKeys_ = false;
	std::unique_ptr<RSACrypto> crypto;
	SHACrypto hasher { 32000 };

	// For accepting connections from players
	std::unique_ptr<APG::AcceptorSocket> playerAcceptor;

	std::vector<std::unique_ptr<PlayerSession>> playerSessions;

	// connections which haven't authenticated themselves yet and so cannot have a session made.
	std::vector<IncomingConnection> incomingConnections;
	std::mutex incomingConnectionsMutex;

	// For accepting connections from map servers which have just spun up
	std::unique_ptr<APG::AcceptorSocket> mapServerAcceptor;

	// A list of connected map servers
	std::vector<APG::Socket> mapServers;

	bool done = false;
	std::thread processingThread;
};

}

#endif /* INCLUDE_SERVER_LOGINSERVER_HPP_ */
