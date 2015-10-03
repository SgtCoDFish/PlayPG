# Based heavily on FindSDL2_image.cmake from the same directory
# The license for this file is the same as for that file.
# Modified by Ashley Davis (https://github.com/SgtCoDFish)

FIND_PATH(SDL2_NET_INCLUDE_DIR SDL_net.h
	HINTS
	${SDL2}
	$ENV{SDL2}
	$ENV{SDL2_NET}
	PATH_SUFFIXES include/SDL2 include SDL2
	i686-w64-mingw32/include/SDL2
	x86_64-w64-mingw32/include/SDL2
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/include/SDL2
	/usr/include/SDL2
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

# Lookup the 64 bit libs on x64
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	FIND_LIBRARY(SDL2_NET_LIBRARY_TEMP
		NAMES SDL2_net
		HINTS
		${SDL2}
		$ENV{SDL2}
		$ENV{SDL2_NET}
		PATH_SUFFIXES lib64 lib
		lib/x64
		x86_64-w64-mingw32/lib
		PATHS
		/sw
		/opt/local
		/opt/csw
		/opt
	)
# On 32bit build find the 32bit libs
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
	FIND_LIBRARY(SDL2_NET_LIBRARY_TEMP
		NAMES SDL2_net
		HINTS
		${SDL2}
		$ENV{SDL2}
		$ENV{SDL2_NET}
		PATH_SUFFIXES lib
		lib/x86
		i686-w64-mingw32/lib
		PATHS
		/sw
		/opt/local
		/opt/csw
		/opt
	)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

SET(SDL2_NET_FOUND "NO")
	IF(SDL2_NET_LIBRARY_TEMP)
	# Set the final string here so the GUI reflects the final state.
	SET(SDL2_NET_LIBRARY ${SDL2_NET_LIBRARY_TEMP} CACHE STRING "Where the SDL2_net Library can be found")
	# Set the temp variable to INTERNAL so it is not seen in the CMake GUI
	SET(SDL2_NET_LIBRARY_TEMP "${SDL2_NET_LIBRARY_TEMP}" CACHE INTERNAL "")
	SET(SDL2_NET_FOUND "YES")
ENDIF(SDL2_NET_LIBRARY_TEMP)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_NET REQUIRED_VARS SDL2_NET_LIBRARY SDL2_NET_INCLUDE_DIR)

