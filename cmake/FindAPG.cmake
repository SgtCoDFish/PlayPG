# APG_INCLUDE_DIR
# APG_DEBUG_LIBRARY
# APG_LIBRARY
# APG_FOUND

find_path(APG_INCLUDE_DIR NAMES APG/APG.hpp
          DOC "The APG include directory"
)

find_library(APG_DEBUG_LIBRARY NAMES APG-d
          DOC "The APG debug library"
)

find_library(APG_LIBRARY NAMES APG
	  DOC "The APG release library"
)

if(APG_DEBUG_LIBRARY OR APG_LIBRARY)
  set(APG_FOUND TRUE)
else()
  set(APG_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)

if (APG_DEBUG_LIBRARY AND APG_LIBRARY)
	message("Found APG: Debug and Release")
	find_package_handle_standard_args(APG DEFAULT_MSG APG_DEBUG_LIBRARY APG_LIBRARY APG_INCLUDE_DIR)
	mark_as_advanced(APG_INCLUDE_DIR APG_DEBUG_LIBRARY APG_LIBRARY)
elseif (APG_DEBUG_LIBRARY)
message("Found APG: Debug only")
	find_package_handle_standard_args(APG DEFAULT_MSG APG_DEBUG_LIBRARY APG_INCLUDE_DIR)
	mark_as_advanced(APG_INCLUDE_DIR APG_DEBUG_LIBRARY)
elseif (APG_LIBRARY)
	message("Found APG: Release only")
	find_package_handle_standard_args(APG DEFAULT_MSG APG_LIBRARY APG_INCLUDE_DIR)
	mark_as_advanced(APG_INCLUDE_DIR APG_LIBRARY)
endif ()
