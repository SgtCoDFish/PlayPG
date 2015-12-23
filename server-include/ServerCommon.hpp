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

#ifndef INCLUDE_SERVER_SERVERCOMMON_HPP_
#define INCLUDE_SERVER_SERVERCOMMON_HPP_

#include <cstdint>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <random>

#include <APG/core/Random.hpp>
#include <APG/APGNet.hpp>

#include <mysql_driver.h>
#include <mysql_connection.h>

#include "net/Opcodes.hpp"

namespace PlayPG {

enum class ServerType {
	LOGIN_SERVER,
	WORLD_SERVER
};

enum class DatabaseType {
	MYSQL
};

class ServerDetails {
public:
	explicit ServerDetails(const std::string &friendlyName, const std::string &hostName, uint16_t port,
	        ServerType serverType);
	~ServerDetails() = default;

	const std::string friendlyName; // a user-friendly name for the server

	const std::string hostName; // a web address, possibly an IP
	const uint16_t port;

	const ServerType serverType;
};

class DatabaseDetails {
public:
	explicit DatabaseDetails(const std::string &hostName, uint16_t port, const std::string &username,
	        const std::string &password, DatabaseType databaseType = DatabaseType::MYSQL);
	~DatabaseDetails() = default;

	const DatabaseType databaseType;

	const std::string hostName;
	const uint16_t port;

	const std::string fullHostName;

	const std::string userName;
	const std::string password;
};

class Server {
public:
	explicit Server(const ServerDetails &serverDetails, const DatabaseDetails &databaseDetails,
	        const std::unordered_map<opcode_type_t, OpcodeDetails>& acceptedOpcodeTypes);
	virtual ~Server() = default;

	const ServerDetails serverDetails;
	const DatabaseDetails databaseDetails;

	virtual void run() = 0;

protected:
	bool isOpcodeAccepted(const opcode_type_t &opcode);

	// These methods are to enable different types of socket to be used depending on platform
	// e.g. to return a NativeAcceptorSocket where SDL is not available but an SDLAcceptorSocket otherwise.
	std::unique_ptr<APG::AcceptorSocket> getAcceptorSocket(const uint16_t port, bool autoListen = false, uint32_t bufferSize_ = BB_DEFAULT_SIZE);

	sql::mysql::MySQL_Driver * driver;
	std::unique_ptr<sql::Connection> mysqlConnection;

	std::unordered_map<opcode_type_t, OpcodeDetails> acceptedOpcodes;

	std::mt19937_64 mersenneTwister;
	APG::Random random;
};

}

#endif /* INCLUDE_SERVER_SERVERCOMMON_HPP_ */
