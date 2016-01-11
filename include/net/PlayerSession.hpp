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

#ifndef INCLUDE_NET_PLAYERSESSION_HPP_
#define INCLUDE_NET_PLAYERSESSION_HPP_

#include <cstdint>

#include <atomic>

#include <APG/core/Random.hpp>
#include <APG/APGNet.hpp>

namespace PlayPG {

class PlayerSession final {
public:
	static std::string generateSessionKey(const std::string &username, const uint_fast64_t &key);

	/**
	 * Create a new player session with the given username and socket.
	 * If random is nullptr, a new random number generator will be created for this session.
	 */
	explicit PlayerSession(const uint64_t &playerID, const std::string &username, std::unique_ptr<APG::Socket> &&socket,
	        APG::Random<uint_fast64_t> &random);
	~PlayerSession() = default;

	const uint64_t playerID;
	const std::string username;
	const std::string sessionKey;

	uint64_t guid;
	std::unique_ptr<APG::Socket> socket;

	bool authenticated = false;

private:
	static std::atomic<uint64_t> nextGUID;
};

}

#endif /* INCLUDE_NET_PLAYERSESSION_HPP_ */
