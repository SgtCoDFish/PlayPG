#ifndef ODB_PLAYER_HPP_
#define ODB_PLAYER_HPP_

#include <cstddef>

#include "PlayPGODB.hpp"

namespace PlayPG {

#pragma db object table("players")
class Player {
public:
	explicit Player(const std::string &username_, const std::string &password_, const std::string &salt_) :
			        id { 0 },
			        username { username_ },
			        password { password_ },
			        salt { salt_ } {
	}

#pragma db id
	uint64_t id;

#pragma db column("email")
	std::string username;
	std::string password;
	std::string salt;

private:
	ODB_FRIEND

	Player() :
			        id { 0 },
			        username { "" },
			        password { "" },
			        salt { "" } {
	}
};

#pragma db view object(Player)
struct PlayerCount {
#pragma db column("count(*)")
	std::size_t count;
};

}

#endif /* ODB_PLAYER_HPP_ */
