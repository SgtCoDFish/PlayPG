# Copyright (c) 2014 Andrew Kelley
# This file is MIT licensed.
# See http://opensource.org/licenses/MIT

# TMXPARSER_INCLUDE_DIR
# TMXPARSER_LIBRARY
# TMXPARSER_FOUND

find_path(TMXPARSER_INCLUDE_DIR NAMES tmxparser/Tmx.h
          DOC "The tmxparser include directory"
)

find_library(TMXPARSER_LIBRARY NAMES tmxparser
          DOC "The tmxparser library"
)

find_library(TMXPARSER_DEBUG_LIBRARY NAMES tmxparser-d
          DOC "The tmxparser-debug library"
)

if(TMXPARSER_LIBRARY OR TMXPARSER_DEBUG_LIBRARY)
  set(TMXPARSER_FOUND TRUE)
else()
  set(TMXPARSER_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TMXPARSER DEFAULT_MSG TMXPARSER_LIBRARY TMXPARSER_INCLUDE_DIR)

mark_as_advanced(TMXPARSER_INCLUDE_DIR TMXPARSER_LIBRARY)

if (TMXPARSER_DEBUG_LIBRARY AND TMXPARSER_LIBRARY)
    message("Found tmxparser:\n\tDebug   (${TMXPARSER_DEBUG_LIBRARY})\n\tRelease (${TMXPARSER_LIBRARY})")
    find_package_handle_standard_args(tmxparser DEFAULT_MSG TMXPARSER_DEBUG_LIBRARY TMXPARSER_LIBRARY TMXPARSER_INCLUDE_DIR)
    mark_as_advanced(TMXPARSER_INCLUDE_DIR TMXPARSER_DEBUG_LIBRARY TMXPARSER_LIBRARY)
elseif (TMXPARSER_DEBUG_LIBRARY)
    message("Found tmxparser: Debug only (${TMXPARSER_DEBUG_LIBRARY})")
    find_package_handle_standard_args(tmxparser DEFAULT_MSG TMXPARSER_DEBUG_LIBRARY TMXPARSER_INCLUDE_DIR)
    mark_as_advanced(TMXPARSER_INCLUDE_DIR TMXPARSER_DEBUG_LIBRARY)
elseif (TMXPARSER_LIBRARY)
    message("Found tmxparser: Release only (${TMXPARSER_LIBRARY})")
    find_package_handle_standard_args(tmxparser DEFAULT_MSG TMXPARSER_LIBRARY TMXPARSER_INCLUDE_DIR)
    mark_as_advanced(TMXPARSER_INCLUDE_DIR TMXPARSER_LIBRARY)
endif ()