cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name
project(Noggit)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBARY_OUTPUT_PATH bin)

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
	  # adds for library repo
	  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/lib/")
	  # freetype2
	  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/freetype2/lib/")
	  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/freetype2/include/freetype2/" ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/freetype2/include/")
	  #glew
	  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Glew/lib/Release/Win32/")
	  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Glew/include/")
	  #storm lib
	  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/DebugAD/" "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/ReleaseAD/" "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/Release/")
	  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/include/")	
	  #SDL
	  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/SDL/lib/x86/")
	  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/SDL/include/")
      #boost
	  SET(IncludeDirectories ${IncludeDirectories} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
	  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/libs/")
	  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")		  
	  #endif()
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

OPTION(NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF)

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF )
IF( NOGGIT_LOGTOCONSOLE )
  MESSAGE( STATUS "And writing log to console instead of log.txt" )
  ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

includePlattform("postfind")

# Find revision ID and hash of the sourcetree
include( "${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake" )
SET(IncludeDirectories ${IncludeDirectories} "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp")

# And do the job.
INCLUDE_DIRECTORIES( ${IncludeDirectories} )
ADD_EXECUTABLE( noggit WIN32 MACOSX_BUNDLE ${SourceFiles} ${ResFiles})
TARGET_LINK_LIBRARIES( noggit ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_LIBRARY} )

includePlattform("pack")
