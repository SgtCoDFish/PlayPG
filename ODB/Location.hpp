/*
 * Copyright (c) 2015-16 See AUTHORS file.
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

#ifndef ODB_LOCATION_HPP_
#define ODB_LOCATION_HPP_

#include <cstdint>

#include <string>

#include "PlayPGODB.hpp"

namespace PlayPG {

#pragma db object table("locations")
class Location {
public:
	explicit Location(const std::string &locationName_, const std::string &knownFileName_,
	        const std::string &knownMD5Hash_) :
			        id { 0u },
			        locationName { locationName_ },
			        knownFileName { knownFileName_ },
			        knownMD5Hash { knownMD5Hash_ },
			        versionMajor { 0u },
			        versionMinor { 0u },
			        versionPatch { 0u } {
	}

#pragma db id auto
	uint64_t id;

	std::string locationName;

	std::string knownFileName;
	std::string knownMD5Hash;

	uint16_t versionMajor;
	uint16_t versionMinor;
	uint16_t versionPatch;

private:
	ODB_FRIEND

	explicit Location() :
			        id { 0u },
			        locationName { "" },
			        knownFileName { "" },
			        knownMD5Hash { "" },
			        versionMajor { 0u },
			        versionMinor { 0u },
			        versionPatch { 0u } {
	}
};

}

#endif /* ODB_LOCATION_HPP_ */
