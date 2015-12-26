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

#include <APG/core/APGeasylogging.hpp>

#include "net/Crypto.hpp"

namespace PlayPG {

Crypto::Crypto() :
		        keyPair { make_rsa_ptr(::RSA_generate_key(KEY_LENGTH, 3, nullptr, nullptr)) },
		        pubKeyBIO { make_bio_ptr(::BIO_new(BIO_s_mem())) },
		        priKeyBIO { make_bio_ptr(::BIO_new(BIO_s_mem())) } {
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

std::string Crypto::encryptString(const std::string &str) {

}

std::string Crypto::decryptString(const std::string &encStr) {

}

bio_ptr make_bio_ptr(BIO *bio) {
	return std::unique_ptr<BIO, void (*)(BIO*)>(bio, ::BIO_free_all);
}

rsa_ptr make_rsa_ptr(RSA *rsa) {
	return std::unique_ptr<RSA, void (*)(RSA *)>(rsa, ::RSA_free);
}

}
