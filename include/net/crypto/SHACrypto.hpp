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

#ifndef INCLUDE_NET_CRYPTO_SHACRYPTO_HPP_
#define INCLUDE_NET_CRYPTO_SHACRYPTO_HPP_

#include <cstdint>

#include <memory>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <iomanip>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "CryptoCommon.hpp"

namespace PlayPG {

class SHACrypto final {
public:
	constexpr static const uint16_t DIGEST_BYTES = 32;
	constexpr static const uint32_t DEFAULT_SALT_BYTES = 16;

	explicit SHACrypto(uint64_t iterationCount);
	~SHACrypto() = default;

	/**
	 * Hash a given password into a SHA256 array, using the given salt.
	 *
	 * Hashing is repeated iterationCount_ times.
	 *
	 * @return the hashed password.
	 */
	std::array<uint8_t, DIGEST_BYTES> hashPasswordSHA256(const std::string &password, const std::vector<uint8_t> &salt);

	/**
	 * Generates a secure random salt of the given length and returns it as a vector.
	 */
	std::vector<uint8_t> generateSalt(uint32_t bytes = DEFAULT_SALT_BYTES);

	std::array<uint8_t, DIGEST_BYTES> stringToSHA256(const std::string &str);
	std::vector<uint8_t> stringToSalt(const std::string &str);

	std::string sha256ToString(const std::array<uint8_t, DIGEST_BYTES> &sha256) {
		return bytesToString(sha256.cbegin(), sha256.cend());
	}

	std::string saltToString(const std::vector<uint8_t> &salt) {
		return bytesToString(salt.cbegin(), salt.cend());
	}

	/**
	 * Convert an STL-style container of uint8_t values to a hex string
	 * where each byte is represented as a 2-character uppercase hex value.
	 */
	template<typename Iter> std::string bytesToString(Iter start, Iter end) {
		std::ostringstream ss;

		ss << std::hex << std::setfill('0') << std::uppercase;

		while (start != end) {
			ss << std::setw(2) << static_cast<int>(*start++);
		}

		return ss.str();
	}

private:
	uint64_t iterationCount_;
};

}

#endif /* INCLUDE_NET_CRYPTO_SHACRYPTO_HPP_ */
