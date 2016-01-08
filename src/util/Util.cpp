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

#include <cassert>

#include <iostream>
#include <sstream>
#include <iomanip>

#include "util/Util.hpp"

namespace PlayPG {

std::string ByteArrayUtil::byteArrayToString(const uint8_t * array, size_t count) {
	std::stringstream ss;

	for (size_t i = 0; i < count; ++i) {
		ss << std::hex << std::setw(2) << std::setfill('0') << (static_cast<uint16_t>(array[i]) & 0xFF);
	}

	return ss.str();
}

std::string ByteArrayUtil::byteVectorToString(const std::vector<uint8_t> &vec) {
	std::stringstream ss;

	for (const auto &c : vec) {
		ss << std::hex << std::setw(2) << std::setfill('0') << (static_cast<uint16_t>(c) & 0xFF);
	}

	return ss.str();
}

std::vector<uint8_t> ByteArrayUtil::hexStringToByteVector(const std::string &str) {
	assert(str.size() % 2 == 0);

	std::vector<uint8_t> ret;
	ret.reserve(str.size() / 2);

	for (std::string::size_type i = 0; i < str.size(); i += 2) {
		const auto subString = str.substr(i, 2);
		ret.emplace_back(static_cast<uint8_t>(std::strtoul(subString.c_str(), nullptr, 16) & 0xFF));

	}

	return ret;
}

}
