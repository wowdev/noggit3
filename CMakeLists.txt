# This file is part of Noggit3, licensed under GNU General Public License (version 3).

cmake_minimum_required(VERSION 3.3)
cmake_policy (SET CMP0057 NEW) # "Support new IN_LIST if() operator."
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name
project(Noggit)

option (NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF)
option (NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF)
option (NOGGIT_OPENGL_ERROR_CHECK "Enable OpenGL error check ?" ON)
option (USE_SQL "Enable sql uid save ? (require mysql installed)" OFF)
option (VALIDATE_OPENGL_PROGRAMS "Validate Opengl programs" ON)

include ("cmake/add_compiler_flag_if_supported.cmake")

add_global_compiler_flag_if_supported (-fcolor-diagnostics)

add_noggit_compiler_flag_if_supported (-Werror=reorder)
add_noggit_compiler_flag_if_supported (-Werror=conditional-uninitialized)
add_noggit_compiler_flag_if_supported (-Werror=unused)
add_noggit_compiler_flag_if_supported (-Werror=unused-parameter)
add_noggit_compiler_flag_if_supported (-Werror=unused-private-field)
add_noggit_compiler_flag_if_supported (-Werror=missing-variable-declarations)
add_noggit_compiler_flag_if_supported (-Werror=missing-prototypes)
add_noggit_compiler_flag_if_supported (-Werror=deprecated)
add_noggit_compiler_flag_if_supported (-Werror=missing-braces)
add_noggit_compiler_flag_if_supported (-Werror=inconsistent-missing-override)
add_noggit_compiler_flag_if_supported (-Werror=parentheses)
add_noggit_compiler_flag_if_supported (-Werror=shift-sign-overflow)
add_noggit_compiler_flag_if_supported (-Werror=string-plus-int)
add_noggit_compiler_flag_if_supported (-Werror=self-assign-field)

# covered by CMAKE_CXX_STANDARD
add_global_compiler_flag_if_supported (-Wno-c++98-compat)
add_global_compiler_flag_if_supported (-Wno-c++98-compat-pedantic)

# covered by compilers used
add_global_compiler_flag_if_supported (-Wno-gnu-anonymous-struct)
add_global_compiler_flag_if_supported (-Wno-variadic-macros)
add_global_compiler_flag_if_supported (-Wno-vla)
add_global_compiler_flag_if_supported (-Wno-vla-extension)
add_global_compiler_flag_if_supported (-Wno-zero-length-array)
add_global_compiler_flag_if_supported (-Wno-gnu-zero-variadic-macro-arguments)
add_global_compiler_flag_if_supported (-Wno-nested-anon-types)
add_global_compiler_flag_if_supported (-Wno-four-char-constants)

# we assume that our constructors and destructors do not access other global state
add_noggit_compiler_flag_if_supported (-Wno-exit-time-destructors)
add_noggit_compiler_flag_if_supported (-Wno-global-constructors)

# is fine with GNU, required due to our libstdc
add_noggit_compiler_flag_if_supported (-Wno-disabled-macro-expansion)

# we can live with the compilation unit containing the vtable not being fixed
add_noggit_compiler_flag_if_supported (-Wno-weak-vtables)
add_noggit_compiler_flag_if_supported (-Wno-weak-template-vtables)

# __DATE__ and __TIME__ not being reproducible is exactly why they exist.
add_noggit_compiler_flag_if_supported (-Wno-date-time)

# we don't care for a few bytes
add_noggit_compiler_flag_if_supported (-Wno-padded)

# msvc++ mangles struct/class into name, thus symbols may be called differently
# with a bad forward-decl. we want compilation to fail, not linking.
add_global_compiler_flag_if_supported (/we4099)
add_global_compiler_flag_if_supported (-Werror=mismatched-tags)

# yes, we intend to use multi-character character constants
add_global_compiler_flag_if_supported (-Wno-multichar)

# declaration of 'identifier' hides previous:
add_noggit_compiler_flag_if_supported (/we4456) # local declaration
add_noggit_compiler_flag_if_supported (/we4457) # function parameter
add_noggit_compiler_flag_if_supported (/we4458) # class members
add_noggit_compiler_flag_if_supported (/we4459) # global declaration
add_noggit_compiler_flag_if_supported (-Werror=shadow)

if( NOGGIT_ALL_WARNINGS )
  MESSAGE( STATUS "Spilling out mass warnings." )
  add_noggit_compiler_flag_if_supported (-W)
  add_noggit_compiler_flag_if_supported (-Wall)
  add_noggit_compiler_flag_if_supported (-Wextra)
  add_noggit_compiler_flag_if_supported (-Weverything)
  add_noggit_compiler_flag_if_supported (/W4)
  add_noggit_compiler_flag_if_supported (/Wall)
endif( NOGGIT_ALL_WARNINGS )

# better exception handling for visual studio, particularly for the asynchronous stuff
add_global_compiler_flag_if_supported (/EHa)
# multi core building for visual studio
add_global_compiler_flag_if_supported (/MP)

add_global_compiler_flag_if_supported_config (/Ob2 Release RelWithDebInfo) # inline any suitable functions
add_global_compiler_flag_if_supported_config (/Oi Release RelWithDebInfo)  # enable intrasic functions
add_global_compiler_flag_if_supported_config (/Ot Release RelWithDebInfo)  # favor fast code
add_global_compiler_flag_if_supported_config (/GL Release RelWithDebInfo)  # whole program optimization
add_global_compiler_flag_if_supported_config (-flto Release RelWithDebInfo)
add_global_compiler_flag_if_supported_config (-O3 Release RelWithDebInfo)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
# todo: why?
set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBARY_OUTPUT_PATH bin)

if(VALIDATE_OPENGL_PROGRAMS)
  add_definitions ( -DVALIDATE_OPENGL_PROGRAMS)

  if(APPLE)
    message(WARNING "Noggit will most likely not work on a mac with option VALIDATE_OPENGL_PROGRAMS enabled.")
  endif()
endif()

# todo: we should probably always require that to avoid an incomplete
# installation with shared libs. it is static on windows luckily.
if (APPLE OR WIN32)
  set (Boost_USE_STATIC_LIBS ON)
endif()

if (WIN32)
  if ("cxx_variable_templates" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    # todo: msvc++ does not like this in new versions. just drop
    # support for old msvc and omit this line?
    add_definitions (-D__cpp_variable_templates=201304)
  endif()
endif()

FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( Boost 1.60 COMPONENTS thread filesystem system unit_test_framework REQUIRED )
FIND_PACKAGE( stormlib REQUIRED )
find_package (Qt5 COMPONENTS Widgets OpenGL OpenGLExtensions)

if (USE_SQL)
  find_library(MYSQL_LIBRARY
    NAMES libmysql)
  find_library(MYSQLCPPCONN_LIBRARY
    NAMES mysqlcppconn)
  find_path (MYSQLCPPCONN_INCLUDE
    NAMES cppconn/driver.h)

  if (MYSQL_LIBRARY AND MYSQLCPPCONN_LIBRARY AND MYSQLCPPCONN_INCLUDE)
    add_definitions ( -DUSE_MYSQL_UID_STORAGE )

    set ( mysql_sources
          src/mysql/mysql.cpp
        )

    set ( mysql_headers
          src/mysql/mysql.h
        )
    source_group("mysql"  FILES ${mysql_sources} ${mysql_headers})
  else()
     message (FATAL_ERROR "MySQL lib or connector not found")
  endif()
endif()

add_subdirectory (src/external/qt-color-widgets)

# Add the found include directories to our include list.
include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/include/")

# Log to console for easier debugging.
IF( NOGGIT_LOGTOCONSOLE )
  MESSAGE( STATUS "And writing log to console instead of log.txt" )
  ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

# Disable opengl error log
IF(NOT NOGGIT_OPENGL_ERROR_CHECK )
  MESSAGE( STATUS "OpenGL error check disabled." )
  ADD_DEFINITIONS( -DNOGGIT_DO_NOT_CHECK_FOR_OPENGL_ERRORS )
ENDIF()

if (APPLE)
  # todo: sounds bogus
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenGL DEFAULT_MSG OPENGL_LIBRARIES OPENGL_INCLUDE_DIR)
elseif (WIN32)
  add_global_compiler_flag_if_supported (/vmg)
  add_compile_definitions (NOMINMAX)

  # mark 32 bit executables large address aware so they can use > 2GB address space
  if(CMAKE_SIZEOF_VOID_P MATCHES 4)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
    message(STATUS "- MSVC: Enabled large address awareness!")
  endif()

  set(ResFiles media/res.rc)
endif()

# And do the job.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src" )

set ( noggit_root_sources
      src/noggit/AsyncLoader.cpp
      src/noggit/Brush.cpp
      src/noggit/ChunkWater.cpp
      src/noggit/cursor_render.cpp
      src/noggit/DBC.cpp
      src/noggit/DBCFile.cpp
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
      src/noggit/world_model_instances_storage.cpp
      src/noggit/world_tile_update_queue.cpp
    )

set ( noggit_ui_sources
      src/noggit/ui/About.cpp
      src/noggit/ui/clickable_label.cpp
      src/noggit/ui/CurrentTexture.cpp
      src/noggit/ui/CursorSwitcher.cpp
      src/noggit/ui/DetailInfos.cpp
      src/noggit/ui/FlattenTool.cpp
      src/noggit/ui/font_awesome.cpp
      src/noggit/ui/font_noggit.cpp
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
      src/noggit/ui/ZoneIDBrowser.cpp
      src/noggit/ui/main_window.cpp
      src/noggit/ui/minimap_widget.cpp
      src/noggit/ui/shader_tool.cpp
      src/noggit/ui/terrain_tool.cpp
      src/noggit/ui/uid_fix_window.cpp
      src/noggit/ui/texture_palette_small.cpp
      src/noggit/ui/TextureList.cpp
    )

set ( math_sources
      src/math/bounding_box.cpp
      src/math/frustum.cpp
      src/math/matrix_4x4.cpp
      src/math/ray.cpp
      src/math/vector_2d.cpp
    )

set ( opengl_sources
      src/opengl/context.cpp
      src/opengl/primitives.cpp
      src/opengl/shader.cpp
      src/opengl/texture.cpp
    )

if (NOT APPLE)
  set_property (SOURCE "src/opengl/context.cpp" PROPERTY
    COMPILE_DEFINITIONS NOGGIT_OPENGL_SUPPORTS_CPU_INDICES_IN_DRAW_ELEMENTS)
endif()

set ( util_sources
      src/util/exception_to_string.cpp
    )

set ( noggit_root_headers
      src/noggit/Animated.h
      src/noggit/AsyncLoader.h
      src/noggit/AsyncObject.h
      src/noggit/Brush.h
      src/noggit/camera.hpp
      src/noggit/ChunkWater.hpp
      src/noggit/cursor_render.hpp
      src/noggit/DBC.h
      src/noggit/DBCFile.h
      src/noggit/Log.h
      src/noggit/MPQ.h
      src/noggit/map_enums.hpp
      src/noggit/MapChunk.h
      src/noggit/MapHeaders.h
      src/noggit/MapTile.h
      src/noggit/MapView.h
      src/noggit/Misc.h
      src/noggit/Model.h
      src/noggit/ModelHeaders.h
      src/noggit/ModelInstance.h
      src/noggit/ModelManager.h
      src/noggit/Particle.h
      src/noggit/Selection.h
      src/noggit/Sky.h
      src/noggit/TextureManager.h
      src/noggit/TileWater.hpp
      src/noggit/WMO.h
      src/noggit/WMOInstance.h
      src/noggit/World.h
      src/noggit/alphamap.hpp
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
      src/noggit/world_model_instances_storage.hpp
      src/noggit/world_tile_update_queue.hpp
    )

set ( noggit_ui_headers
      src/noggit/ui/About.h
      src/noggit/ui/clickable_label.hpp
      src/noggit/ui/CurrentTexture.h
      src/noggit/ui/CursorSwitcher.h
      src/noggit/ui/DetailInfos.h
      src/noggit/ui/FlattenTool.hpp
      src/noggit/ui/font_awesome.hpp
      src/noggit/ui/font_noggit.hpp
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
      src/noggit/ui/ZoneIDBrowser.h
      src/noggit/ui/main_window.hpp
      src/noggit/ui/minimap_widget.hpp
      src/noggit/ui/shader_tool.hpp
      src/noggit/ui/terrain_tool.hpp
      src/noggit/ui/uid_fix_window.hpp
      src/noggit/ui/texture_palette_small.hpp
      src/noggit/ui/TextureList.hpp
    )

set ( math_headers
      src/math/bounding_box.hpp
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
      src/opengl/context.hpp
      src/opengl/primitives.hpp
      src/opengl/scoped.hpp
      src/opengl/shader.fwd.hpp
      src/opengl/shader.hpp
      src/opengl/texture.hpp
      src/opengl/types.hpp
    )

set ( shaders
      src/glsl/m2_vert.glsl
      src/glsl/m2_frag.glsl
      src/glsl/m2_box_vert.glsl
      src/glsl/m2_box_frag.glsl
      src/glsl/particle_vert.glsl
      src/glsl/particle_frag.glsl
      src/glsl/ribbon_vert.glsl
      src/glsl/ribbon_frag.glsl
      src/glsl/terrain_vert.glsl
      src/glsl/terrain_frag.glsl
      src/glsl/wmo_vert.glsl
      src/glsl/wmo_frag.glsl
      src/glsl/liquid_vert.glsl
      src/glsl/liquid_frag.glsl
      src/glsl/mfbo_vert.glsl
      src/glsl/mfbo_frag.glsl
      src/glsl/cursor_vert.glsl
      src/glsl/cursor_frag.glsl
      src/glsl/horizon_vert.glsl
      src/glsl/horizon_frag.glsl
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
  src/noggit/MapView.h
  src/noggit/bool_toggle_property.hpp
  src/noggit/ui/terrain_tool.hpp
  src/noggit/ui/TexturePicker.h
  src/noggit/ui/TexturingGUI.h
  src/noggit/ui/Water.h
  src/noggit/ui/ZoneIDBrowser.h
  src/noggit/ui/clickable_label.hpp
  src/noggit/ui/minimap_widget.hpp
  src/noggit/ui/uid_fix_window.hpp
  src/noggit/ui/widget.hpp
  src/noggit/ui/texture_palette_small.hpp
  src/noggit/ui/TextureList.hpp
  src/noggit/ui/main_window.hpp
  src/noggit/unsigned_int_property.hpp
  src/noggit/ui/CurrentTexture.h
)
qt5_wrap_cpp (moced ${headers_to_moc} ${headers_to_moc})

source_group("noggit"  FILES ${noggit_root_sources} ${noggit_root_headers})
source_group("noggit\\ui"  FILES ${noggit_ui_sources} ${noggit_ui_headers})
source_group("opengl"  FILES ${opengl_sources} ${opengl_headers})
source_group("math"  FILES ${math_sources} ${math_headers})
source_group("external"  FILES ${external_sources} ${external_headers})
source_group("os"  FILES ${os_sources} ${os_headers})
source_group("util"  FILES ${util_sources})
source_group("glsl"  FILES ${shaders})

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
                  ${util_sources}
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
                  ${shaders}
                )
target_compile_options (noggit PRIVATE ${NOGGIT_CXX_FLAGS})

TARGET_LINK_LIBRARIES (noggit
  ${OPENGL_LIBRARIES}
  stormlib::stormlib
  Boost::thread
  Boost::filesystem
  Boost::system
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::OpenGLExtensions
  ColorWidgets-qt5
)

set (_noggit_revision_output_dir "${CMAKE_BINARY_DIR}/revision_output")
set (_noggit_revision_template_file "${CMAKE_SOURCE_DIR}/cmake/revision.h.in")
set (_noggit_revision_output_file "${_noggit_revision_output_dir}/revision.h")
set (_noggit_revision_state_file "${CMAKE_BINARY_DIR}/revision.state")
set (_noggit_revision_script_file "${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake")

include_directories ("${_noggit_revision_output_dir}")

find_package (Git)
if (GIT_FOUND)
  add_custom_target (update_git_revision
    ALL
    DEPENDS "${_noggit_revision_template_file}"
            "${_noggit_revision_script_file}"
    BYPRODUCTS "${_noggit_revision_output_file}"
               "${_noggit_revision_state_file}"
    COMMAND ${CMAKE_COMMAND}
              -D_noggit_revision_template_file="${_noggit_revision_template_file}"
              -D_noggit_revision_output_file="${_noggit_revision_output_file}"
              -D_noggit_revision_state_file="${_noggit_revision_state_file}"
              -DGIT_EXECUTABLE="${GIT_EXECUTABLE}"
              -P "${_noggit_revision_script_file}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

  add_dependencies (noggit update_git_revision)
else()
  message (WARNING "Failed to find a Git executable, will NOT produce a "
    "useful version string. Crash logs will be useless. Do NOT distribute.")

  set (NOGGIT_GIT_VERSION_STRING "UNKNOWN")
  configure_file ("${_noggit_revision_template_file}"
                  "${_noggit_revision_output_file}" @ONLY)
endif()

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

if (NOT APPLE)
  install (TARGETS noggit DESTINATION .)
  if (WIN32)
    install (FILES $<TARGET_PDB_FILE:noggit> DESTINATION . OPTIONAL)
  endif()

  # todo: Qt5{Core,Gui,OpenGL,Widgets}.dll
  # todo: platforms/q{windows,direct2d}.dll
  # todo: if shared: dependencies!

  #set(CPACK_GENERATOR "ZIP;NSIS")
  ## \todo Re-add proper version, if we ever use cpack again.
  #SET(CPACK_BUNDLE_NAME "Noggit_3")
  #SET(CPACK_PACKAGE_FILE_NAME "Noggit")
  #SET(CPACK_PACKAGE_VERSION "3")
  #include(CPack)
else()
  # Also link against libz for static's sake.
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
  INSTALL(TARGETS noggit
    BUNDLE DESTINATION ${CMAKE_CURRENT_BINARY_DIR} COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
  )

  INCLUDE(InstallRequiredSystemLibraries)

  SET(APPS "${CMAKE_CURRENT_BINARY_DIR}/noggit.app")

  SET(DIRS ${OPENGL_LIBRARY_DIR} ${STORMLIB_LIBRARY_DIR} ${Boost_LIBRARY_DIR})
  INSTALL(CODE " include(BundleUtilities) fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\" " COMPONENT Runtime)

  SET(CPACK_BUNDLE_NAME "Noggit 3")
  SET(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/media/Info.plist")
  SET(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/media/noggit.icns")
  SET(CPACK_DMG_VOLUME_NAME "Noggit 3")
  SET(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/media/noggit.png")
  SET(CPACK_GENERATOR "DragNDrop")
  SET(CPACK_PACKAGE_FILE_NAME "Noggit 3")
  INCLUDE(CPack)
endif()

add_library (noggit-math STATIC
  "src/math/matrix_4x4.cpp"
  "src/math/vector_2d.cpp"
)
add_library (noggit::math ALIAS noggit-math)
target_compile_options (noggit-math PRIVATE ${NOGGIT_CXX_FLAGS})

include (CTest)
enable_testing()

# boost::unit_test_framework, when linked using a shared library,
# needs to be explicitly told to generate `main()` as well. CMake's
# FindBoost does not do that, so detect what type of library it gave
# us and fix up that flag if needed.
# todo: what is the state of this on windows?
if ( TARGET Boost::unit_test_framework
    AND Boost_UNIT_TEST_FRAMEWORK_LIBRARY MATCHES ".*\.so[0-9\.]*$")
  get_property (_current_options
    TARGET Boost::unit_test_framework
    PROPERTY INTERFACE_COMPILE_DEFINITIONS)
  set (_boost_test_shared_link_option BOOST_TEST_DYN_LINK)
  if (NOT _boost_test_shared_link_option IN_LIST _current_options)
    set_property (TARGET Boost::unit_test_framework APPEND
      PROPERTY INTERFACE_COMPILE_DEFINITIONS ${_boost_test_shared_link_option})
  endif()
endif()

add_executable (math-vector_2d.test test/math/vector_2d.cpp)
target_compile_definitions (math-vector_2d.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_compile_options (math-vector_2d.test PRIVATE ${NOGGIT_CXX_FLAGS})
target_link_libraries (math-vector_2d.test Boost::unit_test_framework noggit::math)
add_test (NAME math-vector_2d COMMAND $<TARGET_FILE:math-vector_2d.test>)

add_executable (math-trig.test test/math/trig.cpp)
target_compile_definitions (math-trig.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_compile_options (math-trig.test PRIVATE ${NOGGIT_CXX_FLAGS})
target_link_libraries (math-trig.test Boost::unit_test_framework noggit::math)
add_test (NAME math-trig COMMAND $<TARGET_FILE:math-trig.test>)

add_executable (math-matrix_4x4.test test/math/matrix_4x4.cpp)
target_compile_definitions (math-matrix_4x4.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_compile_options (math-matrix_4x4.test PRIVATE ${NOGGIT_CXX_FLAGS})
target_link_libraries (math-matrix_4x4.test Boost::unit_test_framework noggit::math)
add_test (NAME math-matrix_4x4 COMMAND $<TARGET_FILE:math-matrix_4x4.test>)
