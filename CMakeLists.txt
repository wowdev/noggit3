cmake_minimum_required (VERSION 2.8)
cmake_policy (SET CMP0015 OLD)

project (Noggit)

# Additional search paths for find_package.
# Set this to more paths you windows guys need.
if (WIN32)
  set ( CMAKE_INCLUDE_PATH
        "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/include/"
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/include/"
        "C:/Program Files (x86)/glew-1.5.7/include/"
        "C:/Program Files (x86)/StormLib/src/"
      )
  set ( CMAKE_LIBRARY_PATH
        "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/lib/"
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/lib/"
        "C:/Program Files (x86)/StormLib/bin/"
      )
endif( WIN32 )

# Our own FindModule.cmake scripts.
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

find_package (Freetype REQUIRED)
find_package (OpenGL REQUIRED)
find_package (GLEW REQUIRED)
find_package (StormLib REQUIRED)
find_package (Qt4 4.8 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

if (APPLE)
  # Also link against libz for static's sake.
  set (FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES} z)
endif (APPLE)

# Hacky boost finding.
set (Boost_USE_STATIC_LIBS ON)

if (WIN32)
  set (Boost_FIND_QUIETLY ON)
  find_package (Boost)
  set (Boost_FIND_QUIETLY OFF)
  set (BOOST_ROOT ${Boost_INCLUDE_DIR})
endif (WIN32)

find_package (Boost COMPONENTS thread filesystem system REQUIRED)

if (WIN32)
  string ( REGEX REPLACE "/[^/]*$"
           ""
           Boost_STRIPPED_LIB_DIR
           "${Boost_THREAD_LIBRARY_DEBUG}"
         )
  link_directories ( ${Boost_STRIPPED_LIB_DIR}
                     ${STORMLIB_LIBRARY_DIR}
                   )
endif (WIN32)

# Qt setup
include (${QT_USE_FILE})

# include paths
include_directories ( ${GLEW_INCLUDE_PATH}
                      ${GLUT_INCLUDE_DIRS}
                      ${FREETYPE_INCLUDE_DIRS}
                      ${STORMLIB_INCLUDES}
                      ${Boost_INCLUDE_DIRS}
                      "${CMAKE_SOURCE_DIR}/include/"
                    )

if (WIN32)
  include_directories ("${CMAKE_SOURCE_DIR}/include/windows/")
endif (WIN32)

# Gather source files.
set ( SourceFiles
      src/noggit/AsyncLoader.cpp
      src/noggit/Brush.cpp
      src/noggit/ConfigFile.cpp
      src/noggit/DBC.cpp
      src/noggit/DBCFile.cpp
      src/noggit/Directory.cpp
      src/noggit/Environment.cpp
      src/noggit/FreeType.cpp
      src/noggit/Frustum.cpp
      src/noggit/Liquid.cpp
      src/noggit/Log.cpp
      src/noggit/MPQ.cpp
      src/noggit/MapChunk.cpp
      src/noggit/MapTile.cpp
      src/noggit/MapView.cpp
      src/noggit/Menu.cpp
      src/noggit/Misc.cpp
      src/noggit/Model.cpp
      src/noggit/ModelInstance.cpp
      src/noggit/ModelManager.cpp
      src/noggit/Noggit.cpp
      src/noggit/Particle.cpp
      src/noggit/Project.cpp
      src/noggit/Selection.cpp
      src/noggit/Settings.cpp
      src/noggit/Shaders.cpp
      src/noggit/Sky.cpp
      src/noggit/TextureManager.cpp
      src/noggit/UIAppInfo.cpp
      src/noggit/UIButton.cpp
      src/noggit/UICheckBox.cpp
      src/noggit/UICloseWindow.cpp
      src/noggit/UICursorSwitcher.cpp
      src/noggit/UIDetailInfos.cpp
      src/noggit/UIDoodadSpawner.cpp
      src/noggit/UIFrame.cpp
      src/noggit/UIGradient.cpp
      src/noggit/UIListView.cpp
      src/noggit/UIMapViewGUI.cpp
      src/noggit/UIMinimizeButton.cpp
      src/noggit/UIModel.cpp
      src/noggit/UIScrollBar.cpp
      src/noggit/UIScrollableFrame.cpp
      src/noggit/UISlider.cpp
      src/noggit/UIStatusBar.cpp
      src/noggit/UIText.cpp
      src/noggit/UITextBox.cpp
      src/noggit/UITexture.cpp
      src/noggit/UITexturePicker.cpp
      src/noggit/UITextureSwitcher.cpp
      src/noggit/UITexturingGUI.cpp
      src/noggit/UIToggleGroup.cpp
      src/noggit/UIToolbar.cpp
      src/noggit/UIToolbarIcon.cpp
      src/noggit/UITreeView.cpp
      src/noggit/UIWindow.cpp
      src/noggit/UIZoneIDBrowser.cpp
      src/noggit/Video.cpp
      src/noggit/WMO.cpp
      src/noggit/WMOInstance.cpp
      src/noggit/World.cpp

      src/noggit/ui/about_widget.cpp
      src/noggit/ui/help_widget.cpp
      src/noggit/ui/minimap_widget.cpp
    )

if (WIN32)
  set (SourceFiles ${SourceFiles} src/windows/StackWalker.cpp)
endif (WIN32)

set ( AllHeaders
      src/noggit/Animated.h
      src/noggit/AsyncLoader.h
      src/noggit/AsyncObject.h
      src/noggit/Brush.h
      src/noggit/ConfigFile.h
      src/noggit/DBC.h
      src/noggit/DBCFile.h
      src/noggit/Directory.h
      src/noggit/Environment.h
      src/noggit/FreeType.h
      src/noggit/Frustum.h
      src/noggit/Liquid.h
      src/noggit/Log.h
      src/noggit/MPQ.h
      src/noggit/Manager.h
      src/noggit/MapChunk.h
      src/noggit/MapHeaders.h
      src/noggit/MapTile.h
      src/noggit/MapView.h
      src/noggit/Matrix.h
      src/noggit/Menu.h
      src/noggit/Misc.h
      src/noggit/Model.h
      src/noggit/ModelHeaders.h
      src/noggit/ModelInstance.h
      src/noggit/ModelManager.h
      src/noggit/Noggit.h
      src/noggit/Particle.h
      src/noggit/Project.h
      src/noggit/Quaternion.h
      src/noggit/Selection.h
      src/noggit/Settings.h
      src/noggit/Shaders.h
      src/noggit/Sky.h
      src/noggit/TextureManager.h
      src/noggit/UIAppInfo.h
      src/noggit/UIButton.h
      src/noggit/UICheckBox.h
      src/noggit/UICloseWindow.h
      src/noggit/UICursorSwitcher.h
      src/noggit/UIDetailInfos.h
      src/noggit/UIDoodadSpawner.h
      src/noggit/UIEventClasses.h
      src/noggit/UIFrame.h
      src/noggit/UIGradient.h
      src/noggit/UIListView.h
      src/noggit/UIMapViewGUI.h
      src/noggit/UIMinimizeButton.h
      src/noggit/UIModel.h
      src/noggit/UIScrollBar.h
      src/noggit/UIScrollableFrame.h
      src/noggit/UISlider.h
      src/noggit/UIStatusBar.h
      src/noggit/UIText.h
      src/noggit/UITextBox.h
      src/noggit/UITexture.h
      src/noggit/UITexturePicker.h
      src/noggit/UITextureSwitcher.h
      src/noggit/UITexturingGUI.h
      src/noggit/UIToggleGroup.h
      src/noggit/UIToolbar.h
      src/noggit/UIToolbarIcon.h
      src/noggit/UITreeView.h
      src/noggit/UIWindow.h
      src/noggit/UIZoneIDBrowser.h
      src/noggit/Vec3D.h
      src/noggit/Video.h
      src/noggit/WMO.h
      src/noggit/WMOInstance.h
      src/noggit/World.h
      src/noggit/errorHandling.h

      src/noggit/ui/about_widget.h
      src/noggit/ui/help_widget.h
      src/noggit/ui/minimap_widget.h
    )

set ( HeadersToMoc
      src/noggit/Noggit.h
      src/noggit/Menu.h
      src/noggit/MapView.h

      src/noggit/ui/about_widget.h
      src/noggit/ui/help_widget.h
      src/noggit/ui/minimap_widget.h
    )

set ( TRANSLATIONS
      translations/noggit_de.ts
    )

set ( FILES_TO_TRANSLATE
      ${AllHeaders}
      ${SourceFiles}
    )


option (NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF)
option (NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF)

if (NOGGIT_LOGTOCONSOLE)
  message (STATUS "And writing log to console instead of log.txt")
  add_definitions (-DDEBUG__LOGGINGTOCONSOLE)
endif (NOGGIT_LOGTOCONSOLE)

if (NOGGIT_ALL_WARNINGS)
  message (STATUS "Spilling out mass warnings.")
  if (WIN32)
    add_definitions (/W4 /Wall /Wp64)
  else (WIN32)
    add_definitions (-W -Wall -Wshadow)
  endif (WIN32)
endif (NOGGIT_ALL_WARNINGS)

# MSVC++ specific defines.
if (WIN32)
    add_definitions (/vmg /D NOMINMAX)
endif (WIN32)

# Find revision ID and hash of the sourcetree
include ("${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake")
include_directories ("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp")

include_directories ( ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}/src
                    )

# Moc, translate, ressource.
qt4_wrap_cpp (moced ${HeadersToMoc} ${HeadersToMoc})
qt4_automoc (${moced})

qt4_create_translation (QM_FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS})
qt4_add_translation (QM ${TRANSLATIONS})
#qt4_add_resources (rccsc ${foo})

if (APPLE)
  set (ResourceFiles "${CMAKE_SOURCE_DIR}/media/noggit.icns")
  set_source_files_properties ( ${ResourceFiles}
                                PROPERTIES
                                MACOSX_PACKAGE_LOCATION
                                Resources
                              )
endif (APPLE)

if (WIN32)
  add_executable ( noggit
                   WIN32
                   ${SourceFiles}
                   ${AllHeaders}
                   ${moced}
                   ${rccsc}
                   ${QM}
                 )
else (WIN32)
  add_executable ( noggit
                   MACOSX_BUNDLE
                   ${SourceFiles}
                   ${ResourceFiles}
                   ${moced}
                   ${rccsc}
                   ${QM}
                 )
endif (WIN32)

target_link_libraries ( noggit
                        ${OPENGL_LIBRARIES}
                        ${FREETYPE_LIBRARIES}
                        ${STORMLIB_LIBRARIES}
                        ${Boost_LIBRARIES}
                        ${GLEW_LIBRARY}
                        ${QT_LIBRARIES}
                      )

if (APPLE)
  set_target_properties ( noggit
                          PROPERTIES
                          MACOSX_BUNDLE_INFO_PLIST
                          "${CMAKE_SOURCE_DIR}/media/Info.plist"
                        )

  # Copy the binary to bin/.
  install(TARGETS noggit
    BUNDLE DESTINATION ${CMAKE_CURRENT_BINARY_DIR} COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

  include (InstallRequiredSystemLibraries)

  set (APPS "${CMAKE_CURRENT_BINARY_DIR}/noggit.app")

  set ( DIRS
      ${OPENGL_LIBRARY_DIR}
      ${FREETYPE_LIBRARY_DIR}
      ${STORMLIB_LIBRARY_DIR}
      ${Boost_LIBRARY_DIR}
      ${GLEW_LIBRARY_DIR}
      )

  install ( CODE
            " include (BundleUtilities)
              fixup_bundle ( \"${APPS}\"
                             \"\"
                             \"${DIRS}\"
                           )
            "
            COMPONENT
            Runtime
          )
endif (APPLE)
