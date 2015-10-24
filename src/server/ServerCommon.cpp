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

#include "server/ServerCommon.hpp"

namespace PlayPG {

ServerDetails::ServerDetails(const std::string &friendlyName_, const std::string &hostName_, uint16_t port_,
        ServerType serverType_) :
		        friendlyName { friendlyName_ },
		        hostName { hostName_ },
		        port { port_ },
		        serverType { serverType_ } {
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

Server::Server(const ServerDetails &serverDetails_, const DatabaseDetails &databaseDetails_,
        const std::unordered_map<OpcodeType, OpcodeDetails>& acceptedOpcodeTypes_) :
		        serverDetails { serverDetails_ },
		        databaseDetails { databaseDetails_ },
		        driver { sql::mysql::get_driver_instance() },
		        mysqlConnection { std::unique_ptr<sql::Connection>(
		                driver->connect(databaseDetails.fullHostName.c_str(), databaseDetails.userName.c_str(),
		                        databaseDetails.password.c_str())) },
		        acceptedOpcodes { acceptedOpcodeTypes_ } {
}

bool Server::isOpcodeAccepted(const OpcodeType &opcode) {
	return acceptedOpcodes.find(opcode) != acceptedOpcodes.end();
}

}
