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

#include <cstdint>

#include <memory>
#include <array>
#include <utility>
#include <string>

#include <openssl/md5.h>

#include "net/PlayerSession.hpp"
#include "util/Util.hpp"

namespace PlayPG {

std::atomic<uint64_t> PlayerSession::nextGUID { 0ull };

PlayerSession::PlayerSession(const uint64_t &playerID_, const std::string &username_,
        std::unique_ptr<APG::Socket> &&socket_, APG::Random<uint_fast64_t> &random) :
		        playerID { playerID_ },
		        username { username_ },
		        sessionKey { PlayerSession::generateSessionKey(username, random.getDiceRoll()) },
		        guid { nextGUID++ },
		        socket { std::move(socket_) } {
}

std::string PlayerSession::generateSessionKey(const std::string &username, const uint_fast64_t &key) {
	std::array<uint8_t, MD5_DIGEST_LENGTH> buffer;
	const auto combinedString = username + std::to_string(key);

	::MD5(reinterpret_cast<const uint8_t *>(combinedString.c_str()), combinedString.size(), buffer.data());

	return ByteArrayUtil::byteArrayToString(buffer.data(), MD5_DIGEST_LENGTH);
}

}
