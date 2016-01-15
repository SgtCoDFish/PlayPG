#ifndef ODB_PLAYER_HPP_
#define ODB_PLAYER_HPP_

#include <cstddef>

#ifdef PLAYPG_BUILD_SERVER
#include <boost/date_time/posix_time/ptime.hpp>
#endif

#include "PlayPGODB.hpp"

namespace PlayPG {

#pragma db object table("players")
#pragma db value(::boost::posix_time::ptime) type("DATETIME")

class Player {
public:
	explicit Player(const std::string &username_, const std::string &password_, const std::string &salt_) :
			        id { 0 },
			        username { username_ },
			        password { password_ },
			        salt { salt_ },
			        languageID { 1 },
			        locked { false } {
	}

#pragma db id
	uint64_t id;

#pragma db column("email")
	std::string username;
	std::string password;
	std::string salt;

#pragma db default(1)
	uint64_t languageID;

#pragma db default(0)
	bool locked;

#ifdef PLAYPG_BUILD_SERVER
	boost::posix_time::ptime joinDate;
	boost::posix_time::ptime lastLogin;
#endif

private:
	ODB_FRIEND;

	Player() :
	Player("", "", "") {
	}
};

#pragma db view object(Player)
struct PlayerCount {
#pragma db column("count(*)")
	std::size_t count;
};

}

#endif /* ODB_PLAYER_HPP_ */
