# APG_INCLUDE_DIR
# APG_DEBUG_LIBRARY
# APG_LIBRARY
# APG_FOUND

if ( NOT DEFINED APG_PATH )
    set(APG_PATH "")
endif ()

find_path(APG_INCLUDE_DIR NAMES APG/APG.hpp
            DOC "The APG include directory"
)

find_library(APG_DEBUG_LIBRARY NAMES APG-d
            HINTS ${APG_PATH}
            DOC "The APG debug library"
)

find_library(APG_LIBRARY NAMES APG
            HINTS ${APG_PATH}
            DOC "The APG release library"
)

if(APG_DEBUG_LIBRARY OR APG_LIBRARY)
  set(APG_FOUND TRUE)
else()
  set(APG_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)

if (APG_DEBUG_LIBRARY AND APG_LIBRARY)
    message("Found APG:\n\tDebug   (${APG_DEBUG_LIBRARY})\n\tRelease (${APG_LIBRARY})")
    find_package_handle_standard_args(APG DEFAULT_MSG APG_DEBUG_LIBRARY APG_LIBRARY APG_INCLUDE_DIR)
    mark_as_advanced(APG_INCLUDE_DIR APG_DEBUG_LIBRARY APG_LIBRARY)
elseif (APG_DEBUG_LIBRARY)
    message("Found APG: Debug only (${APG_DEBUG_LIBRARY})")
    find_package_handle_standard_args(APG DEFAULT_MSG APG_DEBUG_LIBRARY APG_INCLUDE_DIR)
    mark_as_advanced(APG_INCLUDE_DIR APG_DEBUG_LIBRARY)
elseif (APG_LIBRARY)
    message("Found APG: Release only (${APG_LIBRARY})")
    find_package_handle_standard_args(APG DEFAULT_MSG APG_LIBRARY APG_INCLUDE_DIR)
    mark_as_advanced(APG_INCLUDE_DIR APG_LIBRARY)
endif ()
