cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_LEGACY_CYGWIN32 0)

project(PlayPG)

option(EXCLUDE_CLIENT "Should we exclude building the client program?" OFF)
option(EXCLUDE_SERVER "Should we exclude building the server program?" OFF)

option(EXCLUDE_RELEASE "Should we ignore building release versions of the program? Saves compilation time if we're only working with debug builds" OFF)

option(EXCLUDE_GIT "Should we ignore using git to get the latest commit details?" OFF)

set(PlayPG_VERSION_MAJOR 0)
set(PlayPG_VERSION_MINOR 2)
set(PlayPG_VERSION_PATCH 0)

set(PlayPG_VERSION_EXTRA "")

set(PlayPG_VERSION "${PlayPG_VERSION_MAJOR}.${PlayPG_VERSION_MINOR}.${PlayPG_VERSION_PATCH}")

if( PlayPG_VERSION_EXTRA )
	set(PlayPG_VERSION "${PlayPG_VERSION}-${PlayPG_VERSION_EXTRA}")
endif ()

if( NOT EXCLUDE_GIT )
	find_package(Git)
	
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		OUTPUT_VARIABLE PlayPG_GIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
else ()
	set(PlayPG_GIT_HASH "<N/A>")
endif()


message("Configuring PlayPG version ${PlayPG_VERSION}")

configure_file (
	"${PROJECT_SOURCE_DIR}/gen/PlayPGVersion.hpp.in"
	"${PROJECT_BINARY_DIR}/PlayPGVersion.hpp"
)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Boost REQUIRED COMPONENTS program_options)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_net REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

find_package(TmxParser REQUIRED)

find_package(AshleyCPP REQUIRED)
find_package(APG REQUIRED)

file(GLOB_RECURSE PlayPG_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PlayPG_CLIENT_SOURCES ${PROJECT_SOURCE_DIR}/client/*.cpp)
file(GLOB_RECURSE PlayPG_SERVER_SOURCES ${PROJECT_SOURCE_DIR}/server/*.cpp)
file(GLOB_RECURSE PlayPG_HEADERS ${PROJECT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE PlayPG_TEST_ASSETS ${PROJECT_SOURCE_DIR}/assets/*)

set(CMAKE_CXX_COMPILER_ARG1 "-std=c++1y")
set(CMAKE_CXX_FLAGS_BASE "-Wall -Wextra -Wno-unused-parameter")

file(MAKE_DIRECTORY assets)
file(COPY ${PlayPG_TEST_ASSETS} DESTINATION assets)

set(PlayPG_DEBUG_FLAGS "-g3 -O0")
set(PlayPG_RELEASE_FLAGS "-g0 -O3")
add_definitions(-DGLM_FORCE_RADIANS)

include_directories("include" ${Boost_INCLUDE_DIR} ${APG_INCLUDE_DIR} ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${SDL_NET_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${TMXPARSER_INCLUDE_DIR} ${AshleyCPP_INCLUDE_DIR} ${PROJECT_BINARY_DIR})
set(PlayPG_LIBS ${Boost_LIBRARIES} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARY} ${SDL2_NET_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${TMXPARSER_LIBRARY})

if ( NOT EXCLUDE_CLIENT )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${PlayPG_DEBUG_FLAGS}")
	add_executable(PlayPG-d ${PlayPG_SOURCES} ${PlayPG_CLIENT_SOURCES})
	target_link_libraries(PlayPG-d ${APG_DEBUG_LIBRARY} ${AshleyCPP_DEBUG_LIBRARY} ${PlayPG_LIBS})

	if ( NOT EXCLUDE_RELEASE ) 
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${PlayPG_RELEASE_FLAGS}")
		add_executable(PlayPG ${PlayPG_SOURCES} ${PlayPG_CLIENT_SOURCES})
		target_link_libraries(PlayPG ${APG_LIBRARY} ${AshleyCPP_LIBRARY} ${PlayPG_LIBS})
	endif ()
endif ()

if ( NOT EXCLUDE_SERVER )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${PlayPG_DEBUG_FLAGS}")
	add_executable(serverPG-d ${PlayPG_SOURCES} ${PlayPG_SERVER_SOURCES})
	target_link_libraries(serverPG-d ${APG_DEBUG_LIBRARY} ${AshleyCPP_DEBUG_LIBRARY} ${PlayPG_LIBS})

	if ( NOT EXCLUDE_RELEASE ) 
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${PlayPG_RELEASE_FLAGS}")
		add_executable(serverPG ${PlayPG_SOURCES} ${PlayPG_SERVER_SOURCES})
		target_link_libraries(serverPG ${APG_LIBRARY} ${AshleyCPP_LIBRARY} ${PlayPG_LIBS})
	endif ()
endif () 
