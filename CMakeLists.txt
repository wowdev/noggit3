# This file is part of Noggit3, licensed under GNU General Public License (version 3).

cmake_minimum_required (VERSION 2.8.11)
cmake_policy (SET CMP0015 OLD)

project (Noggit)

option(useLibsRepo "Use windows 32 libs repo")
	   
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED true)

# Additional search paths for find_package.
# Set this to more paths you windows guys need.
if (${useLibsRepo})
  set ( CMAKE_INCLUDE_PATH
        "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/include/"
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/include/"
		"../Noggit_QT_libs/StormLib/include/"
      )
  set ( CMAKE_LIBRARY_PATH
        "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/lib/"
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/lib/"
		"../Noggit_QT_libs/StormLib/ReleaseAD/"
		"../Noggit_QT_libs/StormLib/DebugAD/"
      )

	set(BOOST_ROOT "../Noggit_QT_libs/Boost/1.60/")
	set(QT_BINARY_DIR "../Noggit_QT_libs/Qt/5.6/msvc2015/bin/")
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "../Noggit_QT_libs/Qt/5.6/msvc2015/")

endif(${useLibsRepo})

# Our own FindModule.cmake scripts.
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

set(Boost_USE_STATIC_LIBS ON)



find_package (OpenGL REQUIRED)
find_package (Qt5Widgets REQUIRED)
find_package (Qt5OpenGL REQUIRED)
find_package (Qt5LinguistTools REQUIRED)
find_package (Qt5OpenGLExtensions REQUIRED)
find_package (storm REQUIRED)
find_package (Boost REQUIRED COMPONENTS filesystem system)

# include paths
include_directories ( SYSTEM ${GLUT_INCLUDE_DIRS}
                      SYSTEM ${STORM_INCLUDE_DIR}
                      SYSTEM ${Boost_INCLUDE_DIR}
                    )

# Gather source files.
set ( noggit_root_sources
      src/noggit/application.cpp
      src/noggit/blp_texture.cpp
      src/noggit/Brush.cpp
      src/noggit/DBC.cpp
      src/noggit/DBCFile.cpp
      src/noggit/Frustum.cpp
      src/noggit/Liquid.cpp
      src/noggit/Log.cpp
      src/noggit/map_index.cpp
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
    )

set ( noggit_async_sources
      src/noggit/async/loader.cpp
      src/noggit/async/loading_thread.cpp
    )

set ( noggit_mpq_sources
      src/noggit/mpq/archive_manager.cpp
      src/noggit/mpq/archive.cpp
      src/noggit/mpq/file.cpp
    )

set ( noggit_ui_sources
      src/noggit/ui/about_widget.cpp
      src/noggit/ui/settingsDialog.cpp
      src/noggit/ui/help_widget.cpp
      src/noggit/ui/model_spawner.cpp
      src/noggit/ui/minimap_widget.cpp
      src/noggit/ui/cursor_selector.cpp
      src/noggit/ui/DBCEditor.cpp
      src/noggit/ui/projectExplorer.cpp
      src/noggit/ui/textureselecter.cpp
      src/noggit/ui/main_window.cpp
      src/noggit/ui/zoneid_widget.cpp
    )

set ( noggit_root_headers
      src/noggit/application.h
      src/noggit/blp_texture.h
      src/noggit/Animated.h
      src/noggit/Brush.h
      src/noggit/DBC.h
      src/noggit/DBCFile.h
      src/noggit/Frustum.h
      src/noggit/Liquid.h
      src/noggit/Log.h
      src/noggit/map_index.hpp
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
      src/noggit/ModelView.h
      src/noggit/DBCTableModel.h
    )

set ( noggit_async_headers
      src/noggit/async/loader.h
      src/noggit/async/loading_thread.h
      src/noggit/async/object.h
    )

set ( noggit_mpq_headers
      src/noggit/mpq/archive_manager.h
      src/noggit/mpq/archive.h
      src/noggit/mpq/file.h
    )

set ( noggit_ui_headers
      src/noggit/ui/about_widget.h
      src/noggit/ui/settingsDialog.h
      src/noggit/ui/cursor_selector.h
      src/noggit/ui/help_widget.h
      src/noggit/ui/model_spawner.h
      src/noggit/ui/minimap_widget.h
      src/noggit/ui/main_window.hpp
      src/noggit/ui/DBCEditor.h
      src/noggit/ui/projectExplorer.h
      src/noggit/ui/textureselecter.h
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
      src/noggit/ui/main_window.hpp
      src/noggit/ui/zoneid_widget.h
    )


set ( opengl_sources
      src/opengl/call_list.cpp
      src/opengl/context.cpp
      src/opengl/settings_saver.cpp
      src/opengl/texture.cpp
      src/opengl/primitives.cpp
    )

set ( opengl_headers
	    src/opengl/context.h
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

set ( noggit_sources
      ${noggit_root_sources}
      ${noggit_async_sources}
      ${noggit_mpq_sources}
      ${noggit_ui_sources}
    )

set ( noggit_headers
      ${noggit_root_headers}
      ${noggit_async_headers}
      ${noggit_mpq_headers}
      ${noggit_ui_headers}
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
qt5_wrap_cpp (moced ${headers_to_moc} ${headers_to_moc})

qt5_create_translation (QM_FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS})
qt5_add_translation (QM ${TRANSLATIONS})
# qt4_add_resources (rccsc ${foo})

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

	source_group("noggit"  FILES ${noggit_root_sources} ${noggit_root_headers})
  source_group("noggit\\async"  FILES ${noggit_async_sources} ${noggit_async_headers})
  source_group("noggit\\mpq"  FILES ${noggit_mpq_sources} ${noggit_mpq_headers})
  source_group("noggit\\ui"  FILES ${noggit_ui_sources} ${noggit_ui_headers})
	source_group("opengl"  FILES ${opengl_sources} ${opengl_headers})
	source_group("windows"  FILES ${windows_sources} ${windows_headers})
	source_group("helper"  FILES ${helper_sources} ${helper_headers})
	source_group("math"  FILES ${math_sources} ${math_headers})
	source_group("moced"  FILES ${moced} )
	source_group("resources"  FILES ${noggit_rc} )

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
                        Qt5::Widgets
                        Qt5::OpenGL
                        Qt5::OpenGLExtensions
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
    BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

  include (InstallRequiredSystemLibraries)

  get_filename_component (Qt5_HOME "${Qt5Core_DIR}/../../.." REALPATH)

  set (qt_plugins)
  file (GLOB qt_plugin_dirs
    FOLLOW_SYMLINKS
    RELATIVE "${Qt5_HOME}/plugins/"
    "${Qt5_HOME}/plugins/*"
    )

  foreach (qt_plugin_dir ${qt_plugin_dirs})
    file (GLOB qt_plugins_${qt_plugin_dir}
      FOLLOW_SYMLINKS
      RELATIVE "${Qt5_HOME}/plugins/${qt_plugin_dir}"
      "${Qt5_HOME}/plugins/${qt_plugin_dir}/*"
      )
    foreach (qt_plugin ${qt_plugins_${qt_plugin_dir}})
      install (FILES "${Qt5_HOME}/plugins/${qt_plugin_dir}/${qt_plugin}"
        DESTINATION "noggit.app/Contents/plugins/${qt_plugin_dir}"
        )
      list (APPEND qt_plugins "${CMAKE_INSTALL_PREFIX}/noggit.app/Contents/plugins/${qt_plugin_dir}/${qt_plugin}")
    endforeach()
  endforeach()

  install ( CODE
            " include (BundleUtilities)
              fixup_bundle ( \"${CMAKE_INSTALL_PREFIX}/noggit.app\"
                             \"${qt_plugins}\"
                             \"\"
                           )
            "
            COMPONENT
            Runtime
          )
endif (APPLE)
