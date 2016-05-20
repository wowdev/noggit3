# Findstorm.cmake is part of Noggit3, licensed via GNU General Public License (version 3).
# Bernd Lörwald <bloerwald+noggit@googlemail.com>

#find STORM
# STORM_LIBRARIES, the name of the library to link against
# STORM_FOUND, if false, do not try to link
# STORM_INCLUDES,

find_path(STORM_INCLUDE_DIR StormLib.h StormPort.h)
#find_path(STORM_LIBRARY_DIR StormLibRAD.lib)

SET(CMAKE_FIND_LIBRARY_PREFIXES "")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")

find_library(STORM_LIBRARY StormLibRAD.lib)

if( WIN32 )
  find_library(STORM_LIBRARYRAD StormLibRAD.lib)
  find_library(STORM_LIBRARYDAD StormLibDAD.lib)
  set(STORM_LIBRARIES ${STORM_LIBRARYRAD} ${STORM_LIBRARYDAD})
else( WIN32 )
set(STORM_LIBRARIES ${STORM_LIBRARY})
endif( WIN32 )




if(STORM_INCLUDE_DIR AND STORM_LIBRARY)
    SET(STORM_FOUND TRUE)
endif(STORM_INCLUDE_DIR AND STORM_LIBRARY)

set(STORM_INCLUDES ${STORM_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(storm DEFAULT_MSG STORM_LIBRARYRAD STORM_INCLUDE_DIR)

mark_as_advanced(STORM_INCLUDE_DIR STORM_LIBRARYRAD STORM_LIBRARYDAD)
