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
#include <cmath>

#include <memory>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <APG/APG.hpp>
INITIALIZE_EASYLOGGINGPP

#include "Systems.hpp"
#include "server/ServerCommon.hpp"
#include "server/LoginServer.hpp"
#include "server/MapServer.hpp"
#include "net/Packet.hpp"

#include "PlayPGVersion.hpp"

namespace po = boost::program_options;

bool initializeProgramOptions(int argc, char *argv[]);

bool startLoginServer(el::Logger * logger, const po::variables_map& vm);
bool startWorldServer(el::Logger * logger, const po::variables_map& vm);

int main(int argc, char *argv[]) {
	START_EASYLOGGINGPP(argc, argv);

	APG::Game::setupLoggingDefault();
	APG::Game::setLoggerToAPGStyle("ServPG");
	auto logger = el::Loggers::getLogger("ServPG");

	if (!initializeProgramOptions(argc, argv)) {
		return EXIT_SUCCESS;
	}

	APG::SDLGame::initialiseSDL();

	auto inputManager = std::make_unique<APG::SDLInputManager>();

	IPaddress ip;
	TCPsocket socket;

	if (SDLNet_ResolveHost(&ip, nullptr, 10419) == -1) {
		logger->fatal("Couldn't resolve host: %v.", SDLNet_GetError());
		return EXIT_FAILURE;
	}

	socket = SDLNet_TCP_Open(&ip);

	if (socket == nullptr) {
		logger->fatal("Couldn't open server socket: %v.", SDLNet_GetError());
		return EXIT_FAILURE;
	}

	logger->info("Successfully listening on port 10419.");

	TCPsocket accepted;

	while ((accepted = SDLNet_TCP_Accept(socket)) == nullptr) {
	}

	logger->info("Accepted a connection!");

	while (true) {
		PlayPG::Packet packet;

		if (SDLNet_TCP_Recv(accepted, &packet, sizeof(PlayPG::Packet)) <= 0) {
			logger->info("Finished with remote host.");
			break;
		}

		logger->info("Received packet #%v, entity #%v", packet.id, packet.entity);
	}

	APG::SDLGame::shutdownSDL();
	return EXIT_SUCCESS;
}

bool initializeProgramOptions(int argc, char *argv[]) {
	auto logger = el::Loggers::getLogger("ServPG");
	po::options_description generalOptions("General Options");

	generalOptions.add_options()("help", "list available commands") //
	("version", "print version information") //
	("verbose", "enable verbose logging") //
	("v", po::value<uint16_t>()->default_value(1u),
	        "enable verbose logging with value from 0-9 indicating verbosity level");

	po::options_description serverOptions("Server Options");

	serverOptions.add_options() //
	("login-server", po::value<uint16_t>()->implicit_value(10419u),
	        "run a login server listening on the given port (default 10419)") //
	("world-server", po::value<uint16_t>(),
	        "run a world server listening on the given port (default 10420).") //
	("name", po::value<std::string>()->default_value(std::string("PGserver") + std::to_string(std::rand())),
	        "use the given name for the server, defaulting to \"PGserver\" with a random integer");

	po::options_description databaseOptions("Database Options");

	databaseOptions.add_options()("database-server", po::value<std::string>()->default_value(std::string("localhost")),
	        "the database server to connect to") //
	("database-port", po::value<uint16_t>()->default_value(3306u), "the port the database server listens on") //
	("database-username", po::value<std::string>()->default_value(std::string("root")),
	        "the username for the database") //
	("database-password", po::value<std::string>()->required(), "the password for the database");

	po::options_description worldServerOptions("World Server Specific Options");

	worldServerOptions.add_options()("map-dirs", po::value<std::vector<std::string>>(),
	        "A list of directories in which to search for maps.");

	po::options_description allOptions("Allowed Options");

	allOptions.add(serverOptions).add(worldServerOptions).add(databaseOptions).add(generalOptions);

	po::variables_map vm;

	try {
		po::store(po::parse_command_line(argc, argv, allOptions), vm);
		po::notify(vm);
	} catch (std::exception &e) {
		logger->fatal("%v", e.what());
	}

	if (vm.count("help")) {
		std::cout << allOptions;
		return false;
	}

	if (vm.count("version")) {
		logger->info("PlayPG Server Version %v", PlayPG::Version::versionString);
		logger->info("Built with Git hash: %v", PlayPG::Version::gitHash);

		return false;
	}

	bool ret = true;

	if (vm.count("login-server") && vm.count("world-server")) {
		logger->error("Cannot have both --login-server and --world-server; run two processes instead.");
		return false;
	} else if (vm.count("world-server")) {
		ret = startWorldServer(logger, vm);
	} else {
		if (!vm.count("login-server")) {
			logger->info("Note: Defaulting to starting a login server since no choice was specified.");
			logger->info("Run with --help for more information about server options.");
		}

		ret = startLoginServer(logger, vm);
	}

	return ret;
}

bool startLoginServer(el::Logger * logger, const po::variables_map &vm) {
	logger->info("Starting a login server.");

	const uint16_t serverPort = vm["login-server"].as<uint16_t>();
	const uint16_t dbPort =  vm["database-port"].as<uint16_t>();

	if (!APG::NetUtil::validatePort(serverPort)) {
		logger->error("Invalid server port number: %v", serverPort);
		return false;
	}

	if (!APG::NetUtil::validatePort(dbPort)) {
		logger->error("Invalid database port given: %v", dbPort);
		return false;
	}

	if (!vm.count("database-password")) {
		logger->error("No database password given; use --database-password");
		return false;
	}

	const auto dbServer = vm["database-server"].as<std::string>();
	const auto dbUsername = vm["database-username"].as<std::string>();
	const auto dbPassword = vm["database-password"].as<std::string>();

	PlayPG::ServerDetails serverDetails("edmund", "localhost", 10410, PlayPG::ServerType::LOGIN_SERVER);
	PlayPG::DatabaseDetails dbDetails(dbServer, dbPort, dbUsername, dbPassword);

	PlayPG::LoginServer loginServer(serverDetails, dbDetails);

	return true;
}

bool startWorldServer(el::Logger * logger, const po::variables_map &vm) {
	logger->info("Starting a world server.");
	return true;
}

