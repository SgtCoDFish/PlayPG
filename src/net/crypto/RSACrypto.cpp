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

#include "net/crypto/RSACrypto.hpp"

namespace PlayPG {

constexpr const int RSACrypto::KEY_LENGTH;
constexpr const int RSACrypto::KEY_EXPONENT;

RSACrypto::RSACrypto(const std::string &publicKey_, bool log_) :
		        keyPair { make_rsa_ptr(nullptr) },
		        pubKeyBIO { make_bio_ptr(::BIO_new_mem_buf((void *) publicKey_.data(), -1)) },
		        priKeyBIO { make_bio_ptr(nullptr) },
		        pubKeyLength { publicKey_.size() },
		        priKeyLength { 0 },
		        publicKey { publicKey_ },
		        log { log_ },
		        hasPubKey { true },
		        hasPriKey { false } {
	keyPair = make_rsa_ptr(::PEM_read_bio_RSAPublicKey(pubKeyBIO.get(), nullptr, nullptr, nullptr));

	if (keyPair == nullptr) {
		ERR_load_crypto_strings();

		char * err = ERR_error_string(ERR_get_error(), nullptr);

		el::Loggers::getLogger("PlayPG")->fatal("Couldn't load pubkey from string: %v", err);
	} else if (log) {
		el::Loggers::getLogger("PlayPG")->info("Loaded %v byte RSA PubKey from string.", ::RSA_size(keyPair.get()));
	}
}

RSACrypto::RSACrypto(bool log_) :
		        keyPair { make_rsa_ptr(nullptr) },
		        pubKeyBIO { make_bio_ptr(::BIO_new(BIO_s_mem())) },
		        priKeyBIO { make_bio_ptr(::BIO_new(BIO_s_mem())) },
		        log { log_ },
		        hasPubKey { true },
		        hasPriKey { true } {
	if (log) {
		el::Loggers::getLogger("PlayPG")->info("Generating RSA keypair with %v bit key.", KEY_LENGTH);
	}

	keyPair = make_rsa_ptr(::RSA_generate_key(KEY_LENGTH, KEY_EXPONENT, nullptr, nullptr));

	::PEM_write_bio_RSAPrivateKey(priKeyBIO.get(), keyPair.get(), nullptr, nullptr, 0, nullptr, nullptr);
	::PEM_write_bio_RSAPublicKey(pubKeyBIO.get(), keyPair.get());

	priKeyLength = BIO_pending(priKeyBIO.get());
	pubKeyLength = BIO_pending(pubKeyBIO.get());

	auto privateKeyTempBuffer = std::make_unique<char[]>(priKeyLength + 1);
	auto publicKeyTempBuffer = std::make_unique<char[]>(pubKeyLength + 1);

	::BIO_read(priKeyBIO.get(), privateKeyTempBuffer.get(), priKeyLength);
	::BIO_read(pubKeyBIO.get(), publicKeyTempBuffer.get(), pubKeyLength);

	privateKeyTempBuffer[priKeyLength] = '\0';
	publicKeyTempBuffer[pubKeyLength] = '\0';

	privateKey = std::move(std::string(privateKeyTempBuffer.get(), priKeyLength));
	publicKey = std::move(std::string(publicKeyTempBuffer.get(), pubKeyLength));

	el::Loggers::getLogger("PlayPG")->verbose(9, "Generated RSA key pair:\nPRIVATE: %v\n\nPUBLIC: %v", privateKey,
	        publicKey);
}

std::vector<uint8_t> RSACrypto::encryptStringPublic(const std::string &str) {
	const auto bufSize = ::RSA_size(keyPair.get());

	std::vector<uint8_t> vec;

	if (!hasPubKey) {
		el::Loggers::getLogger("PlayPG")->error("Can't encryptStringPublic without a public key.");
		return vec;
	}

	auto buffer = std::make_unique<uint8_t[]>(bufSize);

	const auto encryptedLength = ::RSA_public_encrypt(str.size(), reinterpret_cast<const unsigned char *>(str.data()),
	        buffer.get(), keyPair.get(), RSA_PKCS1_OAEP_PADDING);

	if (encryptedLength == -1) {
		ERR_load_crypto_strings();

		char * err = ERR_error_string(ERR_get_error(), nullptr);

		el::Loggers::getLogger("PlayPG")->error("Couldn't encrypt string: %v", err);
		return vec;
	}

	vec.reserve(bufSize);
	for (auto i = 0; i < bufSize; ++i) {
		vec.emplace_back(buffer[i]);
	}

	return vec;
}

std::string RSACrypto::decryptStringPrivate(const std::vector<uint8_t> &encStr) {
	if (!hasPriKey) {
		el::Loggers::getLogger("PlayPG")->error("Can't decryptStringPrivate without a private key.");
		return "";
	}

	auto buffer = std::make_unique<char[]>(::RSA_size(keyPair.get()));

	const auto decryptedLength = ::RSA_private_decrypt(encStr.size(), encStr.data(),
	        reinterpret_cast<unsigned char *>(buffer.get()), keyPair.get(), RSA_PKCS1_OAEP_PADDING);

	if (decryptedLength == -1) {
		ERR_load_crypto_strings();

		char * err = ERR_error_string(ERR_get_error(), nullptr);

		el::Loggers::getLogger("PlayPG")->error("Couldn't decrypt string: %v", err);
		return "";
	}

	return std::string(buffer.get(), decryptedLength);
}

}
