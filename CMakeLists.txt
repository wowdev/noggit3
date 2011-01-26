CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Project name for .vcproj etc.
PROJECT( Noggit )

# Additional search paths for find_package.
if( WIN32 )
	# Set this to more paths you windows guys need.
	SET( CMAKE_INCLUDE_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/include/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/include/" )
	SET( CMAKE_LIBRARY_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/lib/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/lib/" )
endif( WIN32 )

# Our own FindModule.cmake scripts.
SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" )

FIND_PACKAGE( zlib REQUIRED )
FIND_PACKAGE( Freetype REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )

# Hack as at least my FindOpenGL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS( OpenGL DEFAULT_MSG OPENGL_LIBRARIES OPENGL_INCLUDE_DIR )

# Hacky boost finding.
SET( Boost_USE_STATIC_LIBS  ON )
#SET( Boost_USE_MULTITHREADED OFF )

IF( WIN32 )
  SET( Boost_FIND_QUIETLY ON )
  FIND_PACKAGE( Boost )
  SET( Boost_FIND_QUIETLY OFF )
  SET( BOOST_ROOT ${Boost_INCLUDE_DIR} )
ENDIF( WIN32 )

FIND_PACKAGE( Boost COMPONENTS thread REQUIRED )

IF( WIN32 )
  STRING( REGEX REPLACE "/[^/]*$" "" Boost_STRIPPED_LIB_DIR "${Boost_THREAD_LIBRARY_DEBUG}" )
  LINK_DIRECTORIES( ${Boost_STRIPPED_LIB_DIR} )
ENDIF( WIN32 )

FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
FIND_PACKAGE( SDL REQUIRED )
MARK_AS_ADVANCED( SDL_LIBRARY SDL_INCLUDE_DIR )

# Hack as at least my FindSDL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SDL DEFAULT_MSG SDL_LIBRARY SDL_INCLUDE_DIR )

# Add the found include directories to our include list.
SET( IncludeDirectories ${ZLIB_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR} ${STORMLIB_INCLUDES} ${Boost_INCLUDE_DIRS} )

# Gather source files.
FILE( GLOB_RECURSE HeaderFiles "src/*.h" )
FILE( GLOB_RECURSE SourceFiles "src/*.cpp" )

# Search for stdint.h, present on unix, missing on windows.
IF( WIN32 )
	SET( IncludeDirectories ${IncludeDirectories} "${CMAKE_SOURCE_DIR}/include/win/" )
	SET( SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/win/StackWalker.cpp" )
ENDIF( WIN32 )

# If we are on OSX, we need additional files for SDL to work.
IF( APPLE )
	MESSAGE( STATUS "Also using osx/SDLmain.m" )
	SET( SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/osx/SDLmain.m" )
ENDIF( APPLE )

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE	 "Log to console instead of log.txt?" OFF )

IF( NOGGIT_LOGTOCONSOLE )
	MESSAGE( STATUS "And writing log to console instead of log.txt" )
	ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

OPTION( NOGGIT_ALL_WARNINGS   "Enable all warnings?" OFF )

IF( NOGGIT_ALL_WARNINGS )
  MESSAGE( STATUS "Spilling out mass warnings." )
  IF( WIN32 )
    ADD_DEFINITIONS( /W4 /Wall /Wp64 )
  ELSE( WIN32 )
    ADD_DEFINITIONS( -W -Wall -Wshadow )
  ENDIF( WIN32 )
ENDIF( NOGGIT_ALL_WARNINGS )

# No console window shown on launch.
IF ( WIN32 )
  SET_TARGET_PROPERTIES( noggit PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" )
ENDIF( WIN32 )

# And do the job.
INCLUDE_DIRECTORIES( ${IncludeDirectories} )
IF( APPLE )
	ADD_EXECUTABLE( noggit MACOSX_BUNDLE	${HeaderFiles} ${SourceFiles} "${CMAKE_SOURCE_DIR}/bin/fonts/arial.ttf" "${CMAKE_SOURCE_DIR}/media/noggit.icns" )
	SET_TARGET_PROPERTIES( noggit PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/media/Info.plist" )
	SET_SOURCE_FILES_PROPERTIES( "${CMAKE_SOURCE_DIR}/bin/fonts/arial.ttf" PROPERTIES MACOSX_PACKAGE_LOCATION MacOS/fonts )
	SET_SOURCE_FILES_PROPERTIES( "${CMAKE_SOURCE_DIR}/media/noggit.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
ELSE( APPLE )
	ADD_EXECUTABLE( noggit	${HeaderFiles} ${SourceFiles} )
ENDIF( APPLE )
TARGET_LINK_LIBRARIES( noggit ${ZLIB_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_LIBRARY} )

# Copy the binary to bin/.
INSTALL( PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/noggit" DESTINATION "${CMAKE_SOURCE_DIR}/bin/" )
