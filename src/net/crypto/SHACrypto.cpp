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

#include <cstdlib>
#include <cstring>

#include <APG/core/APGeasylogging.hpp>

#include "net/crypto/SHACrypto.hpp"

namespace PlayPG {

const uint32_t SHACrypto::DEFAULT_SALT_BYTES;

SHACrypto::SHACrypto(uint64_t iterationCount) :
		        iterationCount_ { iterationCount } {
	if (iterationCount_ <= 16000) {
		el::Loggers::getLogger("PlayPG")->warn(
		        "Warning: password hashing is only repeated %v times, which may be insecure. Consider increasing the number as hardware allows.",
		        iterationCount);
	}
}

std::array<uint8_t, SHACrypto::DIGEST_BYTES> SHACrypto::hashPasswordSHA256(const std::string &password, const std::vector<uint8_t> &salt) {
	if (salt.size() < 16) {
		el::Loggers::getLogger("PlayPG")->warn(
		        "Salt used for SHA256 hash with length %v; minimum of 16 bytes is reccommended.", salt.size());
	}

	std::array<uint8_t, SHACrypto::DIGEST_BYTES> ret;

	::PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt.data(), salt.size(), iterationCount_, ::EVP_sha256(),
	        ret.size(), ret.data());

	return ret;
}

std::vector<uint8_t> SHACrypto::generateSalt(uint32_t bytes) {
	std::vector<uint8_t> ret;
	auto buffer = std::make_unique<uint8_t[]>(bytes);

	if (::RAND_bytes(buffer.get(), bytes) == 0) {
		ERR_load_crypto_strings();

		char * err = ERR_error_string(ERR_get_error(), nullptr);

		el::Loggers::getLogger("PlayPG")->error("Couldn't generate salt: %v", err);
		return ret;
	}

	ret.reserve(bytes);
	for (auto i = 0u; i < bytes; ++i) {
		ret.emplace_back(buffer[i]);
	}

	return ret;
}

std::array<uint8_t, SHACrypto::DIGEST_BYTES> SHACrypto::stringToSHA256(const std::string &str) {
	std::array<uint8_t, DIGEST_BYTES> ret;

	if (str.size() < DIGEST_BYTES * 2u) {
		// 2 characters per hex byte, 32 bytes
		el::Loggers::getLogger("PlayPG")->fatal("Can't convert string to SHA256 array; insufficient length string.");
		return ret;
	}

	for (auto i = 0u; i < ret.size(); ++i) {
		const std::string sub = str.substr(i * 2, 2);

		ret[i] = static_cast<uint8_t>(std::strtoul(sub.c_str(), nullptr, 16));
	}

	return ret;
}

std::vector<uint8_t> SHACrypto::stringToSalt(const std::string &str) {
	std::vector<uint8_t> ret;

	if (str.size() % 2 != 0) {
		// 2 characters per hex byte, 32 bytes
		el::Loggers::getLogger("PlayPG")->fatal("Can't convert string to salt array; must have length divisible by 2.");
		return ret;
	}

	for (auto i = 0u; i < (str.length() / 2); ++i) {
		const std::string sub = str.substr(i * 2, 2);

		ret.emplace_back(static_cast<uint8_t>(std::strtoul(sub.c_str(), nullptr, 16)));
	}

	return ret;
}

}

