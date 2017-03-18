# This file is part of Noggit3, licensed under GNU General Public License (version 3).

cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name
project(Noggit)


include (CheckCXXCompilerFlag)
macro (add_compiler_flag_if_supported _VAR _FLAG)
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_FLAG}" _test_variable)
  check_cxx_compiler_flag ("${_FLAG}" ${_test_variable})
  if (${_test_variable})
     set (${_VAR} "${${_VAR}} ${_FLAG}")
  endif()
endmacro()

add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -fcolor-diagnostics)

add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=reorder)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=conditional-uninitialized)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=unused)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=unused-parameter)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=unused-private-field)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=missing-variable-declarations)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=missing-prototypes)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=deprecated)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=missing-braces)

# covered by CMAKE_CXX_STANDARD
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-c++98-compat)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-c++98-compat-pedantic)

# covered by compilers used
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-gnu-anonymous-struct)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-variadic-macros)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-vla)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-vla-extension)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-zero-length-array)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-gnu-zero-variadic-macro-arguments)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-nested-anon-types)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-four-char-constants)

# we assume that our constructors and destructors do not access other global state
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-exit-time-destructors)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-global-constructors)

# is fine with GNU, required due to our libstdc
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-disabled-macro-expansion)

# we can live with the compilation unit containing the vtable not being fixed
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-weak-vtables)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-weak-template-vtables)

# __DATE__ and __TIME__ not being reproducible is exactly why they exist.
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-date-time)

# we don't care for a few bytes
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-padded)

# msvc++ mangles struct/class into name, thus symbols may be called differently
# with a bad forward-decl. we want compilation to fail, not linking.
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4099)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Werror=mismatched-tags)

# yes, we intend to use multi-character character constants
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-multichar)

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
      #storm lib
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/DebugAD/" "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/ReleaseAD/" "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/Release/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/include/")
      #boost
      include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/libs/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
	  endif(WIN32)
  endif(UNIX)
endmacro(includePlattform)

includePlattform("prefind")

FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( Boost COMPONENTS thread filesystem system unit_test_framework test_exec_monitor REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
find_package (Qt5 COMPONENTS Widgets OpenGL OpenGLExtensions)

find_library(MYSQL_LIBRARY
  NAMES libmysql
  HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql")
find_library(MYSQLCPPCONN_LIBRARY
  NAMES mysqlcppconn
  HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql/connector")
find_path (MYSQLCPPCONN_INCLUDE
  NAMES cppconn/driver.h
  HINTS "${CMAKE_SOURCE_DIR}/../Noggit3libs/mysql/connector")

if (MYSQL_LIBRARY AND MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE)
  add_definitions ( -DUSE_MYSQL_UID_STORAGE )

  set ( mysql_sources
        src/mysql/mysql.cpp
      )

  set ( mysql_headers
        src/mysql/mysql.h
      )
  source_group("mysql"  FILES ${mysql_sources} ${mysql_headers})
endif()

add_subdirectory (src/external/qt-color-widgets)

# Add the found include directories to our include list.
include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/include/")

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
include_directories ("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp")

# And do the job.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src" )

set ( noggit_root_sources
      src/noggit/AsyncLoader.cpp
      src/noggit/Brush.cpp
      src/noggit/ChunkWater.cpp
      src/noggit/ConfigFile.cpp
      src/noggit/DBC.cpp
      src/noggit/DBCFile.cpp
      src/noggit/Directory.cpp
      src/noggit/Environment.cpp
      src/noggit/Log.cpp
      src/noggit/MPQ.cpp
      src/noggit/MapChunk.cpp
      src/noggit/MapTile.cpp
      src/noggit/MapView.cpp
      src/noggit/Misc.cpp
      src/noggit/Model.cpp
      src/noggit/ModelInstance.cpp
      src/noggit/ModelManager.cpp
      src/noggit/Particle.cpp
      src/noggit/Project.cpp
      src/noggit/Settings.cpp
      src/noggit/Sky.cpp
      src/noggit/TextureManager.cpp
      src/noggit/TileWater.cpp
      src/noggit/WMO.cpp
      src/noggit/WMOInstance.cpp
      src/noggit/World.cpp
      src/noggit/alphamap.cpp
      src/noggit/application.cpp
      src/noggit/camera.cpp
      src/noggit/error_handling.cpp
      src/noggit/liquid_layer.cpp
      src/noggit/liquid_render.cpp
      src/noggit/map_horizon.cpp
      src/noggit/map_index.cpp
      src/noggit/texture_set.cpp
      src/noggit/uid_storage.cpp
      src/noggit/wmo_liquid.cpp
    )

if(APPLE)
  list (APPEND noggit_root_sources src/noggit/NativeMac.mm)
elseif(UNIX)
  list (APPEND noggit_root_sources src/noggit/NativeLinux.cpp)
elseif(WIN32)
  list (APPEND noggit_root_sources src/noggit/NativeWindows.cpp)
endif()

set ( noggit_ui_sources
      src/noggit/ui/About.cpp
      src/noggit/ui/clickable_label.cpp
      src/noggit/ui/CurrentTexture.cpp
      src/noggit/ui/CursorSwitcher.cpp
      src/noggit/ui/DetailInfos.cpp
      src/noggit/ui/FlattenTool.cpp
      src/noggit/ui/font_awesome.cpp
      src/noggit/ui/Help.cpp
      src/noggit/ui/HelperModels.cpp
      src/noggit/ui/ModelImport.cpp
      src/noggit/ui/ObjectEditor.cpp
      src/noggit/ui/RotationEditor.cpp
      src/noggit/ui/TexturePicker.cpp
      src/noggit/ui/texture_swapper.cpp
      src/noggit/ui/TexturingGUI.cpp
      src/noggit/ui/texturing_tool.cpp
      src/noggit/ui/Toolbar.cpp
      src/noggit/ui/SettingsPanel.cpp
      src/noggit/ui/Water.cpp
      src/noggit/ui/WaterSaveWarning.cpp
      src/noggit/ui/ZoneIDBrowser.cpp
      src/noggit/ui/main_window.cpp
      src/noggit/ui/minimap_widget.cpp
      src/noggit/ui/shader_tool.cpp
      src/noggit/ui/terrain_tool.cpp
      src/noggit/ui/uid_fix_window.cpp
    )

set ( math_sources
      src/math/frustum.cpp
      src/math/matrix_4x4.cpp
      src/math/ray.cpp
      src/math/vector_2d.cpp
    )

set ( opengl_sources
      src/opengl/call_list.cpp
      src/opengl/context.cpp
      src/opengl/primitives.cpp
      src/opengl/shader.cpp
      src/opengl/texture.cpp
    )

set ( external_sources
      src/external/wacom/Utils.cpp
    )

set ( noggit_root_headers
      src/noggit/Animated.h
      src/noggit/AsyncLoader.h
      src/noggit/AsyncObject.h
      src/noggit/Brush.h
      src/noggit/camera.hpp
      src/noggit/ChunkWater.hpp
      src/noggit/ConfigFile.h
      src/noggit/DBC.h
      src/noggit/DBCFile.h
      src/noggit/Directory.h
      src/noggit/Environment.h
      src/noggit/Log.h
      src/noggit/MPQ.h
      src/noggit/MapChunk.h
      src/noggit/MapHeaders.h
      src/noggit/MapTile.h
      src/noggit/MapView.h
      src/noggit/Misc.h
      src/noggit/Model.h
      src/noggit/ModelHeaders.h
      src/noggit/ModelInstance.h
      src/noggit/ModelManager.h
      src/noggit/Native.hpp
      src/noggit/Particle.h
      src/noggit/Project.h
      src/noggit/Selection.h
      src/noggit/Settings.h
      src/noggit/Sky.h
      src/noggit/TextureManager.h
      src/noggit/TileWater.hpp
      src/noggit/WMO.h
      src/noggit/WMOInstance.h
      src/noggit/World.h
      src/noggit/alphamap.hpp
      src/noggit/application.h
      src/noggit/errorHandling.h
      src/noggit/liquid_layer.hpp
      src/noggit/liquid_render.hpp
      src/noggit/map_horizon.h
      src/noggit/map_index.hpp
      src/noggit/multimap_with_normalized_key.hpp
      src/noggit/texture_set.hpp
      src/noggit/tile_index.hpp
      src/noggit/tool_enums.hpp
      src/noggit/uid_storage.hpp
      src/noggit/wmo_liquid.hpp
    )

set ( noggit_ui_headers
      src/noggit/ui/About.h
      src/noggit/ui/clickable_label.hpp
      src/noggit/ui/CurrentTexture.h
      src/noggit/ui/CursorSwitcher.h
      src/noggit/ui/DetailInfos.h
      src/noggit/ui/FlattenTool.hpp
      src/noggit/ui/font_awesome.hpp
      src/noggit/ui/Help.h
      src/noggit/ui/HelperModels.h
      src/noggit/ui/ModelImport.h
      src/noggit/ui/ObjectEditor.h
      src/noggit/ui/RotationEditor.h
      src/noggit/ui/TexturePicker.h
      src/noggit/ui/texture_swapper.hpp
      src/noggit/ui/TexturingGUI.h
      src/noggit/ui/texturing_tool.hpp
      src/noggit/ui/Toolbar.h
      src/noggit/ui/SettingsPanel.h
      src/noggit/ui/Water.h
      src/noggit/ui/WaterSaveWarning.h
      src/noggit/ui/ZoneIDBrowser.h
      src/noggit/ui/main_window.hpp
      src/noggit/ui/minimap_widget.hpp
      src/noggit/ui/shader_tool.hpp
      src/noggit/ui/terrain_tool.hpp
      src/noggit/ui/uid_fix_window.hpp
    )

set ( math_headers
      src/math/constants.hpp
      src/math/frustum.hpp
      src/math/interpolation.hpp
      src/math/matrix_4x4.hpp
      src/math/projection.hpp
      src/math/quaternion.hpp
      src/math/ray.hpp
      src/math/trig.hpp
      src/math/vector_2d.hpp
      src/math/vector_3d.hpp
      src/math/vector_4d.hpp
    )

set ( opengl_headers
      src/opengl/call_list.hpp
      src/opengl/context.hpp
      src/opengl/matrix.hpp
      src/opengl/primitives.hpp
      src/opengl/scoped.hpp
      src/opengl/shader.fwd.hpp
      src/opengl/shader.hpp
      src/opengl/texture.hpp
      src/opengl/types.hpp
    )

set ( external_headers
      src/external/wacom/MSGPACK.H
      src/external/wacom/PKTDEF.H
      src/external/wacom/Utils.h
      src/external/wacom/WINTAB.H
    )

IF(WIN32)
  set ( os_sources
        include/win/StackWalker.cpp
      )

  set ( os_headers
        include/win/StackWalker.h
      )
ENDIF(WIN32)


list (APPEND headers_to_moc
  src/noggit/bool_toggle_property.hpp
  src/noggit/ui/CurrentTexture.h
  src/noggit/ui/TexturePicker.h
  src/noggit/ui/TexturingGUI.h
  src/noggit/ui/clickable_label.hpp
  src/noggit/ui/minimap_widget.hpp
  src/noggit/ui/widget.hpp
)
qt5_wrap_cpp (moced ${headers_to_moc} ${headers_to_moc})

source_group("noggit"  FILES ${noggit_root_sources} ${noggit_root_headers})
source_group("noggit\\ui"  FILES ${noggit_ui_sources} ${noggit_ui_headers})
source_group("opengl"  FILES ${opengl_sources} ${opengl_headers})
source_group("math"  FILES ${math_sources} ${math_headers})
source_group("external"  FILES ${external_sources} ${external_headers})
source_group("os"  FILES ${os_sources} ${os_headers})

qt5_add_resources (compiled_resource_files "resources/resources.qrc")

ADD_EXECUTABLE ( noggit
                  WIN32
                  MACOSX_BUNDLE
                  ${noggit_root_sources}
                  ${noggit_ui_sources}
                  ${opengl_sources}
                  ${math_sources}
                  ${external_sources}
                  ${mysql_sources}
                  ${os_sources}
                  ${noggit_root_headers}
                  ${noggit_ui_headers}
                  ${opengl_headers}
                  ${math_headers}
                  ${external_headers}
                  ${mysql_headers}
                  ${os_headers}
                  ${ResFiles}
                  ${moced}
                  ${compiled_resource_files}
                )

TARGET_LINK_LIBRARIES (noggit
  ${OPENGL_LIBRARIES}
  StormLib
  Boost::thread
  Boost::filesystem
  Boost::system
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::OpenGLExtensions
  ColorWidgets-qt5
)

if(APPLE)
  TARGET_LINK_LIBRARIES (noggit
    "-framework Cocoa"
    "-framework AppKit"
    "-framework Foundation"
  )
endif()

if (MYSQL_LIBRARY AND MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE)
  target_link_libraries (noggit ${MYSQL_LIBRARY} ${MYSQLCPPCONN_LIBRARY})
  target_include_directories (noggit SYSTEM PRIVATE ${MYSQLCPPCONN_INCLUDE})
endif()

if (NOGGIT_LOGTOCONSOLE AND WIN32)
  set_property (TARGET noggit APPEND PROPERTY LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  set_property (TARGET noggit APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:"_CONSOLE">)
endif()

includePlattform("pack")

add_library (noggit-math STATIC
  "src/math/matrix_4x4.cpp"
  "src/math/vector_2d.cpp"
)
add_library (noggit::math ALIAS noggit-math)

include (CTest)
enable_testing()

add_executable (math-vector_2d.test test/math/vector_2d.cpp)
target_compile_definitions (math-vector_2d.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-vector_2d.test Boost::unit_test_framework Boost::test_exec_monitor noggit::math)
add_test (NAME math-vector_2d COMMAND $<TARGET_FILE:math-vector_2d.test>)

add_executable (math-trig.test test/math/trig.cpp)
target_compile_definitions (math-trig.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-trig.test Boost::unit_test_framework Boost::test_exec_monitor noggit::math)
add_test (NAME math-trig COMMAND $<TARGET_FILE:math-trig.test>)

add_executable (math-matrix_4x4.test test/math/matrix_4x4.cpp)
target_compile_definitions (math-matrix_4x4.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-matrix_4x4.test Boost::unit_test_framework Boost::test_exec_monitor noggit::math)
add_test (NAME math-matrix_4x4 COMMAND $<TARGET_FILE:math-matrix_4x4.test>)
