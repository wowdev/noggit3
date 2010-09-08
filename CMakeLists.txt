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

FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
FIND_PACKAGE( SDL REQUIRED )

# Hack as at least my FindSDL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SDL DEFAULT_MSG SDL_LIBRARY SDL_INCLUDE_DIR )

# Add the found include directories to our include list.
SET( IncludeDirectories ${ZLIB_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR} ${STORMLIB_INCLUDES} )

# Gather source files.

FILE( GLOB_RECURSE HeaderFiles "src/*.h" )
FILE( GLOB_RECURSE SourceFiles "src/*.cpp" )

# Search for stdint.h, present on unix, missing on windows.
#INCLUDE( CheckIncludeFiles )
#CHECK_INCLUDE_FILES( stdint.h HAS_STDINT_H )

# IF( NOT HAS_STDINT_H )
  SET( HeaderFiles ${HeaderFiles} "${CMAKE_SOURCE_DIR}/include/win/stdint.h" )
# ENDIF( NOT HAS_STDINT_H )

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
TARGET_LINK_LIBRARIES( noggit ${ZLIB_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${GLEW_LIBRARY} )

# Copy the binary to bin/.
INSTALL( PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/noggit" DESTINATION "${CMAKE_SOURCE_DIR}/bin/" )