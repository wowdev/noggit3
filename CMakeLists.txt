cmake_minimum_required(VERSION 2.8)

# Project name
project(Noggit)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

macro(includePlattform SUFFIX)
  if(UNIX)
    if(APPLE)
      include("${CMAKE_SOURCE_DIR}/cmake/apple_${SUFFIX}.cmake")
    else(APPLE)
      include("${CMAKE_SOURCE_DIR}/cmake/linux_${SUFFIX}.cmake")
    endif(APPLE)
  else(UNIX)
    if(WIN32)
      include("${CMAKE_SOURCE_DIR}/cmake/win32_${SUFFIX}.cmake")
    endif(WIN32)
  endif(UNIX)
endmacro(includePlattform)

#find all src files
file(GLOB SourceFiles "src/*.h" "src/*.cpp")

includePlattform("prefind")

FIND_PACKAGE( Freetype REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( Boost COMPONENTS thread filesystem system REQUIRED )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
FIND_PACKAGE( SDL REQUIRED )

# Hack as at least my FindSDL does not do this itself.
MARK_AS_ADVANCED( SDL_LIBRARY SDL_INCLUDE_DIR )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SDL DEFAULT_MSG SDL_LIBRARY SDL_INCLUDE_DIR )

# Add the found include directories to our include list.
SET(IncludeDirectories ${GLEW_INCLUDE_PATH} ${GLUT_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR} ${STORMLIB_INCLUDES} ${Boost_INCLUDE_DIRS})
SET(IncludeDirectories ${IncludeDirectories} "${CMAKE_SOURCE_DIR}/include/")

includePlattform("postfind")

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF )
IF( NOGGIT_LOGTOCONSOLE )
  MESSAGE( STATUS "And writing log to console instead of log.txt" )
  ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

# warnings?
OPTION( NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF )
IF( NOGGIT_ALL_WARNINGS )
  MESSAGE( STATUS "Spilling out mass warnings." )
  IF( WIN32 )
    ADD_DEFINITIONS( /W4 /Wall /Wp64 )
  ELSE( WIN32 )
    ADD_DEFINITIONS( -W -Wall -Wshadow )
  ENDIF( WIN32 )
ENDIF( NOGGIT_ALL_WARNINGS )

# Find revision ID and hash of the sourcetree
include( "${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake" )
SET(IncludeDirectories ${IncludeDirectories} "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp")

# And do the job.
INCLUDE_DIRECTORIES( ${IncludeDirectories} )
ADD_EXECUTABLE( noggit WIN32 MACOSX_BUNDLE ${SourceFiles} )
TARGET_LINK_LIBRARIES( noggit ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_LIBRARY} )

includePlattform("pack")
