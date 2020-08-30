# This file is part of Noggit3, licensed under GNU General Public License (version 3).

# adds target stormlib::stormlib
# hopes that users installed stormlib properly so that the CONFIG file gets used instead.

find_path (STORM_INCLUDE_DIR StormLib.h StormPort.h)

find_library (_storm_debug_lib NAMES StormLibDAD StormLibDAS StormLibDUD StormLibDUS)
find_library (_storm_release_lib NAMES StormLibRAD StormLibRAS StormLibRUD StormLibRUS)
find_library (_storm_any_lib NAMES storm stormlib StormLib)

set (STORM_LIBRARIES)
if (_storm_debug_lib AND _storm_release_lib)
  list (APPEND STORM_LIBRARIES debug ${_storm_debug_lib} optimized ${_storm_release_lib})
else()
  list (APPEND STORM_LIBRARIES ${_storm_any_lib})
endif()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (stormlib DEFAULT_MSG STORM_LIBRARIES STORM_INCLUDE_DIR)

mark_as_advanced (STORM_INCLUDE_DIR _storm_debug_lib _storm_release_lib _storm_any_lib STORM_LIBRARIES)

add_library (StormLib INTERFACE)
target_link_libraries (StormLib INTERFACE ${STORM_LIBRARIES})
set_property  (TARGET StormLib APPEND PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${STORM_INCLUDE_DIR})
set_property  (TARGET StormLib APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${STORM_INCLUDE_DIR})


if (NOT WIN32)
  find_package (ZLIB REQUIRED)
  find_package (BZip2 REQUIRED)
  target_link_libraries (StormLib INTERFACE ZLIB::ZLIB BZip2::BZip2)
endif()

target_compile_definitions (StormLib INTERFACE STORMLIB_NO_AUTO_LINK)

add_library (stormlib::stormlib ALIAS StormLib)
