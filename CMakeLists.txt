CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

cmake_policy(SET CMP0015 OLD)

# Project name for .vcproj etc.
PROJECT( Noggit )

# Additional search paths for find_package.
if( WIN32 )
  # Set this to more paths you windows guys need.
  SET( CMAKE_INCLUDE_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/include/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/include/" "C:/Program Files (x86)/SDL-1.2.14/include/SDL/" "C:/Program Files (x86)/glew-1.5.7/include/" "C:/Program Files (x86)/StormLib/src/")
  SET( CMAKE_LIBRARY_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/lib/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/lib/" "C:/Program Files (x86)/StormLib/bin/" )
endif( WIN32 )

# Our own FindModule.cmake scripts.
SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" )

FIND_PACKAGE( Freetype REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )

if( APPLE )
# Hack as at least my FindOpenGL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS( OpenGL DEFAULT_MSG OPENGL_LIBRARIES OPENGL_INCLUDE_DIR )
endif( APPLE )

# Hacky boost finding.
SET( Boost_USE_STATIC_LIBS ON )
#SET( Boost_USE_MULTITHREADED OFF )

IF( WIN32 )
  SET( Boost_FIND_QUIETLY ON )
  FIND_PACKAGE( Boost )
  SET( Boost_FIND_QUIETLY OFF )
  SET( BOOST_ROOT ${Boost_INCLUDE_DIR} )
ENDIF( WIN32 )

FIND_PACKAGE( Boost COMPONENTS thread filesystem system REQUIRED )

IF( WIN32 )
  STRING( REGEX REPLACE "/[^/]*$" "" Boost_STRIPPED_LIB_DIR "${Boost_THREAD_LIBRARY_DEBUG}" )
  LINK_DIRECTORIES( ${Boost_STRIPPED_LIB_DIR} ${STORMLIB_LIBRARY_DIR} )
ENDIF( WIN32 )

FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
FIND_PACKAGE( SDL REQUIRED )
MARK_AS_ADVANCED( SDL_LIBRARY SDL_INCLUDE_DIR )

# Hack as at least my FindSDL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SDL DEFAULT_MSG SDL_LIBRARY SDL_INCLUDE_DIR )

# Add the found include directories to our include list.
SET( IncludeDirectories ${GLEW_INCLUDE_PATH} ${GLUT_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR} ${STORMLIB_INCLUDES} ${Boost_INCLUDE_DIRS} )

# Gather source files.
SET( SourceFiles
  src/AsyncLoader.cpp
  src/Brush.cpp
  src/ConfigFile.cpp
  src/DBC.cpp
  src/DBCFile.cpp
  src/Directory.cpp
  src/Environment.cpp
  src/FreeType.cpp
  src/Frustum.cpp
  src/Liquid.cpp
  src/Log.cpp
  src/MPQ.cpp
  src/MapChunk.cpp
  src/MapTile.cpp
  src/MapView.cpp
  src/Menu.cpp
  src/Misc.cpp
  src/Model.cpp
  src/ModelInstance.cpp
  src/ModelManager.cpp
  src/Noggit.cpp
  src/Particle.cpp
  src/Project.cpp
  src/Selection.cpp
  src/Settings.cpp
  src/Shaders.cpp
  src/Sky.cpp
  src/TextureManager.cpp
  src/UIAbout.cpp
  src/UIAppInfo.cpp
  src/UIButton.cpp
  src/UICheckBox.cpp
  src/UICloseWindow.cpp
  src/UIDetailInfos.cpp
  src/UIFrame.cpp
  src/UIGradient.cpp
  src/UIHelp.cpp
  src/UIListView.cpp
  src/UIMapViewGUI.cpp
  src/UIMenuBar.cpp
  src/UIMinimapWindow.cpp
  src/UIMinimizeButton.cpp
  src/UIModel.cpp
  src/UIScrollableFrame.cpp
  src/UIScrollBar.cpp
  src/UISlider.cpp
  src/UIStatusBar.cpp
  src/UIText.cpp
  src/UITextBox.cpp
  src/UITexture.cpp
  src/UITexturePicker.cpp
  src/UITextureSwitcher.cpp
  src/UITexturingGUI.cpp
  src/UIToggleGroup.cpp
  src/UIToolbar.cpp
  src/UIToolbarIcon.cpp
  src/UITreeView.cpp
  src/UIWindow.cpp
  src/UIZoneIDBrowser.cpp
  src/Video.cpp
  src/WMO.cpp
  src/WMOInstance.cpp
  src/World.cpp
)

SET( IncludeDirectories ${IncludeDirectories} "${CMAKE_SOURCE_DIR}/include/" )

# Search for stdint.h, present on unix, missing on windows.
IF( WIN32 )
  SET( IncludeDirectories ${IncludeDirectories} "${CMAKE_SOURCE_DIR}/include/win/" )
  SET( SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/win/StackWalker.cpp" )
  ADD_DEFINITIONS( /vmg /D NOMINMAX )
ENDIF( WIN32 )

# If we are on OSX, we need additional files for SDL to work.
IF( APPLE )
  MESSAGE( STATUS "Also using osx/SDLMain.m" )
  SET( SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/osx/SDLMain.m" )
ENDIF( APPLE )

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF )

IF( NOGGIT_LOGTOCONSOLE )
  MESSAGE( STATUS "And writing log to console instead of log.txt" )
  ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

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
SET( IncludeDirectories ${IncludeDirectories} "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp" )

# And do the job.
INCLUDE_DIRECTORIES( ${IncludeDirectories} )
IF( APPLE )
  SET( ResourceFiles "${CMAKE_SOURCE_DIR}/media/noggit.icns" )
  SET_SOURCE_FILES_PROPERTIES( ${ResourceFiles} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
  SET( SourceFiles ${SourceFiles} ${ResourceFiles} )

  # Also link against libz for static's sake.
  SET( FREETYPE_LIBRARIES  ${FREETYPE_LIBRARIES} z )
ENDIF( APPLE )

ADD_EXECUTABLE( noggit WIN32 MACOSX_BUNDLE ${SourceFiles} )

IF( APPLE )
  SET_TARGET_PROPERTIES( noggit PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/media/Info.plist" )
ENDIF( APPLE )

TARGET_LINK_LIBRARIES( noggit ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_LIBRARY} )

# Copy the binary to bin/.
#INSTALL( PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/noggit" DESTINATION "${CMAKE_SOURCE_DIR}/bin/" )
