cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_LEGACY_CYGWIN32 0)

project(PlayPG)

option(APG_NO_SDL "Is the APG library built without SDL? Can trim down the size of the APG library + server significantly and allow builidng on headless platforms. Implies -DEXCLUDE_CLIENT" OFF)

option(EXCLUDE_CLIENT "Should we exclude building the client program?" OFF)
option(EXCLUDE_SERVER "Should we exclude building the server program?" OFF)

option(EXCLUDE_GIT "Should we ignore using git to get the latest commit details?" OFF)

if ( APG_NO_SDL AND NOT EXCLUDE_CLIENT )
    message("Excluding client as APG_NO_SDL is set.")
    set(EXCLUDE_CLIENT ON)
endif ()

set(PlayPG_VERSION_MAJOR 0)
set(PlayPG_VERSION_MINOR 2)
set(PlayPG_VERSION_PATCH 0)

set(PlayPG_VERSION_EXTRA "")

set(PlayPG_VERSION "${PlayPG_VERSION_MAJOR}.${PlayPG_VERSION_MINOR}.${PlayPG_VERSION_PATCH}")

if ( PlayPG_VERSION_EXTRA )
	set(PlayPG_VERSION "${PlayPG_VERSION}-${PlayPG_VERSION_EXTRA}")
endif ()

if ( CMAKE_BUILD_TYPE MATCHES "Release")
	message("Building in release mode.")
	set (PlayPG_DEBUG FALSE)
else ()
	message("Building in debug mode.")
	set (PlayPG_DEBUG TRUE)
	set (PlayPG_VERSION "${PlayPG_VERSION}-d")
endif ()

if ( NOT EXCLUDE_GIT )
	find_package(Git REQUIRED)
	
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

if ( NOT APG_NO_SDL )
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(SDL2_ttf REQUIRED)
	find_package(SDL2_net REQUIRED)
    
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
endif ()

find_package(APG REQUIRED)

find_package(OpenSSL REQUIRED)

find_package(GLM REQUIRED)
find_package(AshleyCPP REQUIRED)
find_package(rapidjson REQUIRED)

find_package(TmxParser REQUIRED)
find_package(TinyXML2 REQUIRED)
find_package(ZLIB REQUIRED)

add_definitions(-DGLM_FORCE_RADIANS)

file(GLOB_RECURSE PlayPG_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PlayPG_HEADERS ${PROJECT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE PlayPG_TEST_ASSETS ${PROJECT_SOURCE_DIR}/assets/*)

set(OS_LIBS "")

if ( WIN32 )
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif ()

if ( NOT MSVC )
	set(CMAKE_CXX_COMPILER_ARG1 "-std=c++1y")
	set(CMAKE_CXX_FLAGS_BASE "-Wall -Wextra -Wno-unused-parameter -pthread")
	
	set(PlayPG_DEBUG_FLAGS "-g3 -O0")
	set(PlayPG_RELEASE_FLAGS "-g0 -O3")
else ()
    set(OS_LIBS ${OS_LIBS} ws2_32)
	# VS 2015 has a linker issue, this should fix it
	if( MSVC_VERSION GREATER 1800)
		message ("Using legacy STDIO fix for MSVC 15.")
		set(OS_LIBS ${OS_LIBS} legacy_stdio_definitions)
		set(PlayPG_SOURCES ${PlayPG_SOURCES} ${PROJECT_SOURCE_DIR}/MSVC/vs2015_io_fix.cpp)
	endif()
	
	set(CMAKE_CXX_FLAGS_BASE "${CMAKE_CXX_FLAGS_BASE} /EHsc")
	
	set(PlayPG_DEBUG_FLAGS "/Od /Gs")
	set(PlayPG_RELEASE_FLAGS "/Ox")
endif ()

include_directories("include"
                    ${GLM_INCLUDE_DIRS}
                    
                    ${AshleyCPP_INCLUDE_DIR}
                    ${APG_INCLUDE_DIR}
                    
                    ${TMXPARSER_INCLUDE_DIR}
                    ${TINYXML2_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIR}
                    
                    ${RAPIDJSON_INCLUDE_DIRS}
                    
                    ${OPENSSL_INCLUDE_DIR}
                    
                    ${PROJECT_BINARY_DIR}
                    )

set(PlayPG_LIBS ${OS_LIBS}

                ${TMXPARSER_LIBRARY}
                ${TINYXML2_LIBRARIES}
                ${ZLIB_LIBRARIES}
                
                ${OPENSSL_LIBRARIES}
                )
                
if ( NOT APG_NO_SDL )
    include_directories(${SDL2_IMAGE_INCLUDE_DIR}
                        ${SDL2_INCLUDE_DIR}
                        ${SDL2_TTF_INCLUDE_DIR}
                        ${SDL2_MIXER_INCLUDE_DIR}
                        ${SDL2_NET_INCLUDE_DIR}
    
                        ${OPENGL_INCLUDE_DIR}
                        ${GLEW_INCLUDE_DIRS}
                        )
                        
    set(PlayPG_LIBS ${PlayPG_LIBS}
                    ${SDL2_LIBRARY}
                    ${SDL2_IMAGE_LIBRARY}
                    ${SDL2_TTF_LIBRARIES}
                    ${SDL2_MIXER_LIBRARY}
                    ${SDL2_NET_LIBRARY}
                    
                    ${OPENGL_LIBRARIES}
                    ${GLEW_LIBRARIES}
                    )
else ()
    add_definitions(-DAPG_NO_SDL -DAPG_NO_GL)
endif ()

if( PlayPG_DEBUG )
    set(PlayPG_LIBS ${PlayPG_LIBS}
                    ${APG_DEBUG_LIBRARY}
                    ${AshleyCPP_DEBUG_LIBRARY}
                    )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${PlayPG_DEBUG_FLAGS}")
else ()
    set(PlayPG_LIBS ${PlayPG_LIBS}
                    ${APG_LIBRARY}
                    ${AshleyCPP_LIBRARY}
                    )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${PlayPG_RELEASE_FLAGS}")
endif ()

if ( NOT EXCLUDE_CLIENT )
    include (Client.cmake)
endif ()

if ( NOT EXCLUDE_SERVER )
    include (Server.cmake)
endif ()
