CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Project name for .vcproj etc.
PROJECT( Noggit )

# Additional search paths for find_package.
if( WIN32 )
  # Set this to more paths you windows guys need.
  SET( CMAKE_INCLUDE_PATH "C:/Program Files/Microsoft Visual Studio 9.0/VC/include/" "C:/Program Files/Microsoft SDKs/Windows/v6.0A/include/" )
  SET( CMAKE_LIBRARY_PATH "C:/Program Files/Microsoft Visual Studio 9.0/VC/lib/" "C:/Program Files/Microsoft SDKs/Windows/v6.0A/lib/" )
endif( WIN32 )

# Our own FindModule.cmake scripts.
SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" )

FIND_PACKAGE( zlib REQUIRED )
FIND_PACKAGE( Freetype REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )

# Hack as at least my FindOpenGL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS( OpenGL DEFAULT_MSG OPENGL_LIBRARIES OPENGL_INCLUDE_DIR )


# Bernd! Try if this also works for you! If not we must make a TV session to get it to work on all systems.

    # HACK - CMake on Windows refuses to find the thread library unless BOOST_ROOT
	#        is set, even though it can locate the Boost directory tree.
	#        So we first look for base Boost, then set BOOST_ROOT and look again
	#        for Boost Thread specifically.
	
	message(STATUS "-- Boost:")
	set(Boost_USE_STATIC_LIBS OFF)
	set(Boost_USE_MULTITHREADED ON)
	set(Boost_ADDITIONAL_VERSIONS "1.44" "1.44.0" )
	#set(Boost_FIND_QUIETLY ON)
	find_package(Boost)
	if(IS_DIRECTORY "${Boost_INCLUDE_DIR}")
	  message(STATUS "boost headers : ${Boost_INCLUDE_DIR}")
	else(IS_DIRECTORY "${Boost_INCLUDE_DIR}")
	  if(UNIX)
	    message(FATAL_ERROR "Cannot build without Boost Thread library. Please install libboost-thread-dev package or visit www.boost.org.")
	  else(UNIX)
	    message(FATAL_ERROR "Cannot build without Boost Thread library. Please install Boost from www.boost.org.")
	  endif(UNIX)
	endif(IS_DIRECTORY "${Boost_INCLUDE_DIR}")
	#set(Boost_FIND_QUIETLY OFF)
	if(WIN32)
	  set(BOOST_ROOT ${Boost_INCLUDE_DIR})
	endif(WIN32)
	find_package(Boost COMPONENTS thread REQUIRED)
	
	# HACK - Make linking to Boost work on Windows systems.
	string(REGEX REPLACE "/[^/]*$" ""
	  Boost_STRIPPED_LIB_DIR "${Boost_THREAD_LIBRARY_DEBUG}")
	
	if(EXISTS "${Boost_THREAD_LIBRARY_DEBUG}")
	  message(STATUS "boost lib dir : ${Boost_STRIPPED_LIB_DIR}")
	  message(STATUS "thread lib    : ${Boost_THREAD_LIBRARY_DEBUG}")
	else(EXISTS "${Boost_THREAD_LIBRARY_DEBUG}")
	  message(FATAL_ERROR "Cannot build without Boost Thread library. Please install libboost-thread-dev package or visit www.boost.org.")
	endif(EXISTS "${Boost_THREAD_LIBRARY_DEBUG}")
	message(STATUS "")
	
	# HACK - Make linking to Boost work on Windows systems.
	if(WIN32)
	  link_directories(${Boost_STRIPPED_LIB_DIR})
	endif(WIN32)


FIND_PACKAGE( Boost REQUIRED COMPONENTS thread )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
FIND_PACKAGE( SDL REQUIRED )

# Hack as at least my FindSDL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SDL DEFAULT_MSG SDL_LIBRARY SDL_INCLUDE_DIR )

# Add the found include directories to our include list.
SET( IncludeDirectories ${ZLIB_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR} ${STORMLIB_INCLUDES} ${Boost_INCLUDE_DIRS} )

# Gather source files.

FILE( GLOB_RECURSE HeaderFiles "src/*.h" )
FILE( GLOB_RECURSE SourceFiles "src/*.cpp" )

# Search for stdint.h, present on unix, missing on windows.
INCLUDE( CheckIncludeFiles )
CHECK_INCLUDE_FILES( stdint.h HAS_STDINT_H )

MESSAGE( STATUS HAS_STDINT_H: ${HAS_STDINT_H} )
IF( NOT HAS_STDINT_H )
  SET( HeaderFiles ${HeaderFiles} "${CMAKE_SOURCE_DIR}/include/win/stdint.h" )
ENDIF( NOT HAS_STDINT_H )

# If we are on OSX, we need additional files for SDL to work.
IF( APPLE )
  MESSAGE( STATUS "Also using osx/SDLmain.m" )
  SET( SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/osx/SDLmain.m" )
ENDIF( APPLE )

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE   "Log to console instead of log.txt?" OFF )

IF( NOGGIT_LOGTOCONSOLE )
  MESSAGE( STATUS "And writing log to console instead of log.txt" )
  ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

# And do the job.
INCLUDE_DIRECTORIES( ${IncludeDirectories} )
ADD_EXECUTABLE( noggit ${HeaderFiles} ${SourceFiles} )
TARGET_LINK_LIBRARIES( noggit ${ZLIB_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_LIBRARY} )

MESSAGE( STATUS Boost_LIBRARIES: ${Boost_LIBRARIES} )

# Copy the binary to bin/.
INSTALL( PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/noggit" DESTINATION "${CMAKE_SOURCE_DIR}/bin/" )
