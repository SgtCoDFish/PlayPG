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

#include <APG/core/APGeasylogging.hpp>

#include "net/crypto/SHACrypto.hpp"

namespace PlayPG {

SHACrypto::SHACrypto(uint64_t iterationCount) :
		        iterationCount_ { iterationCount } {
	if (iterationCount_ <= 32000) {
		el::Loggers::getLogger("PlayPG")->warn(
		        "Warning: password hashing is only repeated %v times, which may be insecure. Consider increasing the number as hardware allows.",
		        iterationCount);
	}
}

std::array<uint8_t, 64> SHACrypto::hashPasswordSHA512(const std::string &password, const std::vector<uint8_t> &salt) {
	if (salt.size() < 16) {
		el::Loggers::getLogger("PlayPG")->warn("Salt used for SHA512 hash with length %v; minimum of 16 bytes is reccommended.",
		        salt.size());
	}

	std::array<uint8_t, 64> ret;

	::PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt.data(), salt.size(), iterationCount_, ::EVP_sha512(),
	        ret.size(), ret.data());

	return ret;
}

}

