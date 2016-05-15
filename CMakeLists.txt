# CMakeLists.txt is part of Noggit3, licensed via GNU General Publiicense (version 3).
# Bernd Lörwald <bloerwald+noggit@googlemail.com>
# Glararan <glararan@glararan.eu>
# Stephan Biegel <project.modcraft@googlemail.com>
# Tigurius <bstigurius@googlemail.com>

cmake_minimum_required (VERSION 2.8)
cmake_policy (SET CMP0015 OLD)

project (Noggit)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED true)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-absolute-value")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-dangling-else")

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

find_package (OpenGL REQUIRED)
find_package (GLEW REQUIRED)
find_package (Qt4 4.8 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
find_package (storm REQUIRED)
find_package (Boost REQUIRED COMPONENTS filesystem system)

# Qt setup
include (${QT_USE_FILE})

# include paths
include_directories ( ${GLEW_INCLUDE_PATH}
                      ${GLUT_INCLUDE_DIRS}
                      ${STORM_INCLUDES}
                      ${Boost_INCLUDE_DIR}
                      "${CMAKE_SOURCE_DIR}/include/"
                    )

if (WIN32)
  include_directories ("${CMAKE_SOURCE_DIR}/include/windows/")
endif (WIN32)

# Gather source files.
set ( noggit_sources
      src/noggit/application.cpp
      src/noggit/blp_texture.cpp

      src/noggit/Brush.cpp
      src/noggit/DBC.cpp
      src/noggit/DBCFile.cpp
      src/noggit/Frustum.cpp
      src/noggit/Liquid.cpp
      src/noggit/Log.cpp
      src/noggit/MapChunk.cpp
      src/noggit/MapTile.cpp
      src/noggit/MapView.cpp
      src/noggit/Menu.cpp
      src/noggit/Model.cpp
      src/noggit/ModelInstance.cpp
      src/noggit/ModelManager.cpp
      src/noggit/Particle.cpp
      src/noggit/Selection.cpp
      src/noggit/Shaders.cpp
      src/noggit/Sky.cpp
      src/noggit/TextureManager.cpp
      src/noggit/WMO.cpp
      src/noggit/WMOInstance.cpp
      src/noggit/World.cpp
      src/noggit/ModelView.cpp
      src/noggit/DBCTableModel.cpp

      src/noggit/ChunkWater.cpp
      src/noggit/TileWater.cpp

      src/noggit/async/loader.cpp
      src/noggit/async/loading_thread.cpp

      src/noggit/mpq/archive_manager.cpp
      src/noggit/mpq/archive.cpp
      src/noggit/mpq/file.cpp

      src/noggit/ui/about_widget.cpp
      src/noggit/ui/settingsDialog.cpp
      src/noggit/ui/help_widget.cpp
      src/noggit/ui/model_spawner.cpp
      src/noggit/ui/minimap_widget.cpp
      src/noggit/ui/cursor_selector.cpp
      src/noggit/ui/DBCEditor.cpp
      src/noggit/ui/projectExplorer.cpp
      src/noggit/ui/textureselecter.cpp
      src/noggit/ui/editortemplate.cpp
      src/noggit/ui/MainWindow.cpp
      src/noggit/ui/zoneid_widget.cpp
    )

set ( noggit_headers
      src/noggit/application.h
      src/noggit/blp_texture.h

      src/noggit/Animated.h
      src/noggit/Brush.h
      src/noggit/DBC.h
      src/noggit/DBCFile.h
      src/noggit/Frustum.h
      src/noggit/Liquid.h
      src/noggit/Log.h
      src/noggit/Manager.h
      src/noggit/MapChunk.h
      src/noggit/MapHeaders.h
      src/noggit/MapTile.h
      src/noggit/MapView.h
      src/noggit/Menu.h
      src/noggit/Model.h
      src/noggit/ModelHeaders.h
      src/noggit/ModelInstance.h
      src/noggit/ModelManager.h
      src/noggit/Particle.h
      src/noggit/Selection.h
      src/noggit/Shaders.h
      src/noggit/Sky.h
      src/noggit/TextureManager.h
      src/noggit/WMO.h
      src/noggit/WMOInstance.h
      src/noggit/World.h
      src/noggit/errorHandling.h
      src/noggit/ModelView.h
      src/noggit/DBCTableModel.h

      src/noggit/async/loader.h
      src/noggit/async/loading_thread.h
      src/noggit/async/object.h

      src/noggit/mpq/archive_manager.h
      src/noggit/mpq/archive.h
      src/noggit/mpq/file.h

      src/noggit/ui/about_widget.h
      src/noggit/ui/settingsDialog.h
      src/noggit/ui/cursor_selector.h
      src/noggit/ui/help_widget.h
      src/noggit/ui/model_spawner.h
      src/noggit/ui/minimap_widget.h
      src/noggit/ui/DBCEditor.h
      src/noggit/ui/projectExplorer.h
      src/noggit/ui/textureselecter.h
      src/noggit/ui/editortemplate.h
      src/noggit/ui/MainWindow.h
      src/noggit/ui/zoneid_widget.h
    )

set ( noggit_headers_to_moc
      src/noggit/application.h

      src/noggit/Menu.h
      src/noggit/MapView.h
      src/noggit/ModelView.h
      src/noggit/DBCTableModel.h

      src/noggit/async/loader.h
      src/noggit/async/loading_thread.h

      src/noggit/ui/about_widget.h
      src/noggit/ui/settingsDialog.h
      src/noggit/ui/cursor_selector.h
      src/noggit/ui/help_widget.h
      src/noggit/ui/model_spawner.h
      src/noggit/ui/minimap_widget.h
      src/noggit/ui/DBCEditor.h
      src/noggit/ui/projectExplorer.h
      src/noggit/ui/textureselecter.h
      src/noggit/ui/editortemplate.h
      src/noggit/ui/MainWindow.h
      src/noggit/ui/zoneid_widget.h
    )


set ( opengl_sources
      src/opengl/call_list.cpp
      src/opengl/settings_saver.cpp
      src/opengl/texture.cpp
      src/opengl/primitives.cpp
    )

set ( opengl_headers
      src/opengl/call_list.h
      src/opengl/settings_saver.h
      src/opengl/texture.h
      src/opengl/types.h
      src/opengl/primitives.h
      src/opengl/scoped.h
    )


set ( windows_sources
      src/windows/StackWalker.cpp
    )

set ( windows_headers
      src/windows/StackWalker.h
    )


set ( helper_sources
      src/helper/qt/non_recursive_filter_model.cpp
      src/helper/qt/case_insensitive.cpp
    )

set ( helper_headers
      src/helper/qt/non_recursive_filter_model.h
      src/helper/qt/signal_blocker.h
      src/helper/qt/case_insensitive.h
    )


set ( math_sources
      src/math/matrix_4x4.cpp
      src/math/vector_2d.cpp
    )

set ( math_headers
      src/math/bounded_nearest.h
      src/math/constants.h
      src/math/interpolation.h
      src/math/matrix_4x4.h
      src/math/random.h
      src/math/vector_2d.h
      src/math/vector_3d.h
      src/math/vector_4d.h
      src/math/quaternion.h
    )

set ( headers_to_moc
      ${noggit_headers_to_moc}
    )

if (COMPILE_DEFINITIONS_RELEASE)

file (GLOB TRANSLATIONS translations/noggit_*.ts)

set ( FILES_TO_TRANSLATE
      ${noggit_headers}
      ${noggit_sources}
    )

endif (COMPILE_DEFINITIONS_RELEASE)

option (NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF)
option (NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF)
option (NOGGIT_USE_BLS "Use BLS shaders?" OFF)

if (NOGGIT_LOGTOCONSOLE)
  message (STATUS "And writing log to console instead of log.txt")
  add_definitions (-DDEBUG__LOGGINGTOCONSOLE)
endif (NOGGIT_LOGTOCONSOLE)

if (NOGGIT_USE_BLS)
  message (STATUS "using bls shaders")
  add_definitions (-DUSEBLSSHADER)
  add_definitions (-DUSEBLSFILES)
endif (NOGGIT_USE_BLS)

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
    add_definitions (/vmg /D NOMINMAX /wd"4503" /wd"4996")
endif (WIN32)

# Find revision ID and hash of the sourcetree
include ("${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake")

include_directories ( ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}/src
                    )

# Moc, translate, ressource.
qt4_wrap_cpp (moced ${headers_to_moc} ${headers_to_moc})
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
  set ( ResourceFiles
      "${CMAKE_SOURCE_DIR}/media/noggit.rc"
  )
  set ( noggit_rc
      media/noggit.rc
  )


  add_executable ( noggit
                   WIN32
                   ${noggit_sources}
                   ${opengl_sources}
                   ${windows_sources}
                   ${helper_sources}
                   ${math_sources}
                   ${noggit_headers}
                   ${opengl_headers}
                   ${windows_headers}
                   ${helper_headers}
                   ${math_headers}
                   ${noggit_rc}
                   ${moced}
                   ${rccsc}
                   ${QM}
                 )
else (WIN32)
  add_executable ( noggit
                   MACOSX_BUNDLE
                   ${noggit_sources}
                   ${opengl_sources}
                   ${helper_sources}
                   ${math_sources}
                   ${ResourceFiles}
                   ${moced}
                   ${rccsc}
                   ${QM}
                 )
endif (WIN32)

target_link_libraries ( noggit
                        ${OPENGL_LIBRARIES}
                        ${STORM_LIBRARIES}
                        ${GLEW_LIBRARY}
                        ${QT_LIBRARIES}
                        ${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                      )

if (APPLE)
  set_target_properties ( noggit
                          PROPERTIES
                          MACOSX_BUNDLE_ICON_FILE
                          "noggit"
                        )
  set_source_files_properties (media/noggit.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set_target_properties ( noggit
                          PROPERTIES
                          MACOSX_BUNDLE_BUNDLE_NAME
                          "Noggit 3"
                        )
  set_target_properties ( noggit
                          PROPERTIES
                          MACOSX_BUNDLE_BUNDLE_VERSION
                          "3"
                        )
  set_target_properties ( noggit
                          PROPERTIES
                          MACOSX_BUNDLE_GUI_IDENTIFIER
                          "com.noggit.noggit"
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
      ${STORM_LIBRARY_DIR}
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
