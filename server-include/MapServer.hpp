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

#ifndef INCLUDE_SERVER_MAPSERVER_HPP_
#define INCLUDE_SERVER_MAPSERVER_HPP_

#include <string>
#include <vector>
#include <memory>

#include <tmxparser/TmxMap.h>

#include "ServerCommon.hpp"
#include "net/crypto/RSACrypto.hpp"

namespace PlayPG {

class MapServer final : public Server {
public:
	explicit MapServer(const ServerDetails &details, const DatabaseDetails &databaseDetails_,
	        std::vector<std::string> &&maps, const std::string &masterServer_, const uint16_t &masterPort, const std::string &masterPublicKeyFile_,
	        const std::string &masterPrivateKeyFile_);
	virtual ~MapServer() = default;

	virtual void run() override final;

private:
	bool parseMaps(el::Logger * const logger);
	bool registerWithMasterServer();

	const std::vector<std::string> mapPaths;
	std::vector<std::unique_ptr<Tmx::Map>> maps;

	std::unique_ptr<APG::Socket> connectedPlayers;

	const std::string masterServerHostname;
	const uint16_t masterServerPort;

	std::unique_ptr<RSACrypto> masterServerCrypto;
	std::unique_ptr<APG::Socket> masterServerConnection;
};

}

#endif /* INCLUDE_SERVER_MAPSERVER_HPP_ */
