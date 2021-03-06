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

#include <chrono>

#include "ServerCommon.hpp"

#include <odb/database.hxx>

namespace PlayPG {

ServerDetails::ServerDetails(const std::string &friendlyName_, const std::string &hostName_, uint16_t port_,
        ServerType serverType_, const boost::optional<std::vector<std::string>> &maps_,
        const boost::optional<const std::string> &publicKeyFile_,
        const boost::optional<const std::string> &privateKeyFile_) :
		        friendlyName { friendlyName_ },
		        hostName { hostName_ },
		        port { port_ },
		        serverType { serverType_ },
		        maps { maps_ },
		        publicKeyFile { publicKeyFile_ },
		        privateKeyFile { privateKeyFile_ } {
}

DatabaseDetails::DatabaseDetails(const std::string &hostName_, uint16_t port_, const std::string &username_,
        const std::string &password_, DatabaseType databaseType_) :
		        databaseType { databaseType_ },
		        hostName { hostName_ },
		        port { port_ },
		        fullHostName { hostName + ":" + std::to_string(port) },
		        userName { username_ },
		        password { password_ } {

}

Server::Server(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_) :
		        serverDetails { serverDetails_ },
		        databaseDetails { databaseDetails_ },
		        db { Server::getDatabaseConnection(databaseDetails) },
		        mersenneTwister {
		                static_cast<std::mt19937_64::result_type>(static_cast<std::mt19937_64::result_type>(randomDevice())
		                        << 32 | static_cast<std::mt19937_64::result_type>(randomDevice())) },
		        random { mersenneTwister } {
}

std::unique_ptr<APG::Socket> Server::getSocket(const std::string &hostname_, const uint16_t &port_, bool autoConnect_,
        uint32_t bufferSize_) {
#ifndef APG_NO_SDL
	return std::make_unique<APG::SDLSocket>(hostname_, port_, autoConnect_, bufferSize_);
#else
	return std::make_unique<APG::NativeSocket>(hostname_, port_, autoConnect_, bufferSize_);
#endif
}

std::unique_ptr<APG::AcceptorSocket> Server::getAcceptorSocket(const uint16_t port, bool autoListen,
        uint32_t bufferSize_) {
#ifndef APG_NO_SDL
	return std::make_unique<APG::SDLAcceptorSocket>(port, autoListen, bufferSize_);
#else
	return std::make_unique<APG::NativeDualAcceptorSocket>(port, autoListen, bufferSize_);
#endif
}

std::unique_ptr<odb::database> Server::getDatabaseConnection(const DatabaseDetails &details) {
#ifdef DATABASE_MYSQL
	return std::unique_ptr<odb::database>(new odb::mysql::database(details.userName.c_str(), details.password.c_str(), "ppg", details.hostName.c_str(),
	        details.port));
#else
#error "ONLY CONFIGURED FOR MYSQL SO FAR"
#endif
}

}
