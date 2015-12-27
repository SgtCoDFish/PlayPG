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

#ifndef INCLUDE_NET_CRYPTO_HPP_
#define INCLUDE_NET_CRYPTO_HPP_

#include <cstdint>

#include <memory>
#include <string>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

namespace PlayPG {

using bio_ptr = std::unique_ptr<BIO, void(*)(BIO*)>;
bio_ptr make_bio_ptr(BIO *bio);

using rsa_ptr = std::unique_ptr<RSA, void(*)(RSA *)>;
rsa_ptr make_rsa_ptr(RSA *rsa);

class Crypto final {
public:
	constexpr static const int KEY_LENGTH = 4096;
	constexpr static const int KEY_EXPONENT = 3;

	/**
	 * Initialise with a PEM-formatted pubkey (likely received over the network.)
	 * @param publicKey
	 * @param log
	 */
	explicit Crypto(const std::string &publicKey, bool log = false);
	explicit Crypto(bool log = false);
	~Crypto() = default;

	/**
	 * Encrypt the given string with the public key loaded; will require the matching private key
	 * to decrypt later.
	 */
	std::vector<uint8_t> encryptStringPublic(const std::string &str);

	/**
	 * Decrypts the given string using the loaded private key, assuming that it was encrypted with
	 * the matching public key.
	 */
	std::string decryptStringPrivate(const std::vector<uint8_t> &vec);

	std::string getPublicKeyPEM() const {
		return publicKey;
	}

private:
	rsa_ptr keyPair;

	bio_ptr pubKeyBIO;
	bio_ptr priKeyBIO;

	size_t pubKeyLength;
	size_t priKeyLength;

	std::string privateKey;
	std::string publicKey;

	bool log;

	bool hasPubKey;
	bool hasPriKey;
};

}

#endif /* INCLUDE_NET_CRYPTO_HPP_ */
