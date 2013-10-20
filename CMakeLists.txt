#CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

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

IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET( Boost_USE_STATIC_LIBS ON )
ENDIF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
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

SET( SourceFiles
  src/Animated.h
  src/AppState.h
  src/AsyncLoader.cpp
  src/AsyncLoader.h
  src/Alphamap.cpp
  src/Alphamap.h
  src/AsyncObject.h
  src/Brush.cpp
  src/Brush.h
  src/ChunkWater.cpp
  src/ChunkWater.h
  src/ConfigFile.cpp
  src/ConfigFile.h
  src/DBC.cpp
  src/DBC.h
  src/DBCFile.cpp
  src/DBCFile.h
  src/Directory.cpp
  src/Directory.h
  src/Environment.cpp
  src/Environment.h
  src/errorHandling.h
  src/FreeType.cpp
  src/FreeType.h
  src/Frustum.cpp
  src/Frustum.h
  src/Liquid.cpp
  src/Liquid.h
  src/Log.cpp
  src/Log.h
  src/Manager.h
  src/MapChunk.cpp
  src/MapChunk.h
  src/MapHeaders.h
  src/MapIndex.cpp
  src/MapIndex.h
  src/MapTile.cpp
  src/MapTile.h
  src/MapView.cpp
  src/MapView.h
  src/Matrix.h
  src/Menu.cpp
  src/Menu.h
  src/Misc.cpp
  src/Misc.h
  src/Model.cpp
  src/Model.h
  src/ModelHeaders.h
  src/ModelInstance.cpp
  src/ModelInstance.h
  src/ModelManager.cpp
  src/ModelManager.h
  src/MPQ.cpp
  src/MPQ.h
  src/Noggit.cpp
  src/Noggit.h
  src/Particle.cpp
  src/Particle.h
  src/Project.cpp
  src/Project.h
  src/Quaternion.h
  src/StateMachine.cpp
  src/StateMachine.h
  src/Selection.cpp
  src/Selection.h
  src/Settings.cpp
  src/Settings.h
  src/Shaders.cpp
  src/Shaders.h
  src/Sky.cpp
  src/Sky.h
  src/TextureManager.cpp
  src/TextureManager.h
  src/TextureSet.cpp
  src/TextureSet.h
  src/TileWater.cpp
  src/TileWater.h
  src/UIAbout.cpp
  src/UIAbout.h
  src/UIAppInfo.cpp
  src/UIAppInfo.h
  src/UIAlphamap.cpp
  src/UIAlphamap.h
  src/UIButton.cpp
  src/UIButton.h
  src/UICheckBox.cpp
  src/UICheckBox.h
  src/UICloseWindow.cpp
  src/UICloseWindow.h
  src/UICursorSwitcher.cpp
  src/UICursorSwitcher.h
  src/UIDetailInfos.cpp
  src/UIDetailInfos.h
  src/UIDoodadSpawner.cpp
  src/UIDoodadSpawner.h
  src/UIEventClasses.h
  src/UIFrame.cpp
  src/UIFrame.h
  src/UIGradient.cpp
  src/UIGradient.h
  src/UIHelp.cpp
  src/UIHelp.h
  src/UIListView.cpp
  src/UIListView.h
  src/UIMapViewGUI.cpp
  src/UIMapViewGUI.h
  src/UIMenuBar.cpp
  src/UIMenuBar.h
  src/UIMinimapWindow.cpp
  src/UIMinimapWindow.h
  src/UIMinimizeButton.cpp
  src/UIMinimizeButton.h
  src/UIExitWarning.cpp
  src/UIExitWarning.h
  src/UICapsWarning.cpp
  src/UICapsWarning.h
  src/UIWaterSaveWarning.cpp
  src/UIWaterSaveWarning.h
  src/UIWaterTypeBrowser.cpp
  src/UIWaterTypeBrowser.h
  src/UIHelperModels.cpp
  src/UIHelperModels.h
  src/UIModel.cpp
  src/UIModel.h
  src/UIModelImport.cpp
  src/UIModelImport.h
  src/UIScrollableFrame.cpp
  src/UIScrollableFrame.h
  src/UIScrollBar.cpp
  src/UIScrollBar.h
  src/UISlider.cpp
  src/UISlider.h
  src/UIStatusBar.cpp
  src/UIStatusBar.h
  src/UIText.cpp
  src/UIText.h
  src/UITextBox.cpp
  src/UITextBox.h
  src/UITexture.cpp
  src/UITexture.h
  src/UITexturePicker.cpp
  src/UITexturePicker.h
  src/UITextureSwitcher.cpp
  src/UITextureSwitcher.h
  src/UITexturingGUI.cpp
  src/UITexturingGUI.h
  src/UIToggleGroup.cpp
  src/UIToggleGroup.h
  src/UIToolbar.cpp
  src/UIToolbar.h
  src/UIToolbarIcon.cpp
  src/UIToolbarIcon.h
  src/UITreeView.cpp
  src/UITreeView.h
  src/UIWindow.cpp
  src/UIWindow.h
  src/UIWater.cpp
  src/UIWater.h
  src/UIZoneIDBrowser.cpp
  src/UIZoneIDBrowser.h
  src/Vec3D.h
  src/Video.cpp
  src/Video.h
  src/WMO.cpp
  src/WMO.h
  src/WMOInstance.cpp
  src/WMOInstance.h
  src/World.cpp
  src/World.h
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

# Set loglevel to w1 on windows and activate
IF( WIN32 ) 
    add_definitions(/W1) 
	# mark 32 bit executables large address aware so they can use > 2GB address space
	if(CMAKE_SIZEOF_VOID_P MATCHES 4)
	  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
	  message(STATUS "- MSVC: Enabled large address awareness!")
	endif()
endif() 


  
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
  SET( FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES} z )
ENDIF( APPLE )

ADD_EXECUTABLE( noggit WIN32 MACOSX_BUNDLE ${SourceFiles} )

IF( APPLE )
  SET_TARGET_PROPERTIES( noggit PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/media/Info.plist" )
ENDIF( APPLE )

TARGET_LINK_LIBRARIES( noggit ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_LIBRARY} )

IF(APPLE)
  # Copy the binary to bin/.
  INSTALL(TARGETS noggit
    BUNDLE DESTINATION ${CMAKE_CURRENT_BINARY_DIR} COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

  INCLUDE(InstallRequiredSystemLibraries)

  SET(APPS "${CMAKE_CURRENT_BINARY_DIR}/noggit.app")

  SET(DIRS ${OPENGL_LIBRARY_DIR} ${FREETYPE_LIBRARY_DIR} ${SDL_LIBRARY_DIR} ${STORMLIB_LIBRARY_DIR} ${Boost_LIBRARY_DIR} ${GLEW_LIBRARY_DIR})
  INSTALL(CODE "
   include(BundleUtilities)
   fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
   " COMPONENT Runtime)

  SET(CPACK_BUNDLE_NAME "Noggit 3")
  SET(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/media/Info.plist")
  SET(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/media/noggit.icns")
  SET(CPACK_DMG_VOLUME_NAME "Noggit 3")
  SET(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/media/noggit.png")
  SET(CPACK_GENERATOR "DragNDrop")
  SET(CPACK_PACKAGE_FILE_NAME "Noggit 3")
  SET(CPACK_PACKAGE_VERSION "SDL 1.3")
  INCLUDE(CPack)
ENDIF(APPLE)

install(TARGETS noggit DESTINATION bin)
install(DIRECTORY "bin" DESTINATION .)
