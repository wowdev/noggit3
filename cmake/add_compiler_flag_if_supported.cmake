# This file is part of Noggit3, licensed under GNU General Public License (version 3).
# This file is based on CMake's CheckCXXCompilerFlag, distributed
# under BSD 3-Clause. See https://cmake.org/licensing for details.

include_guard (GLOBAL)

## begin copy [[
include(CheckCXXSourceCompiles)
include(CMakeCheckCompilerFlagCommonPatterns)

macro (CHECK_CXX_COMPILER_FLAG _FLAG _RESULT)
  set(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
  set(CMAKE_REQUIRED_DEFINITIONS "${_FLAG}")
## end copy ]]

  # GCC only warns on unknown -Wno-, but errors on -W
  string (SUBSTRING "${_FLAG}" 0 5 _flag_prefix)
  if ("${_flag_prefix}" STREQUAL "-Wno-")
    string (REPLACE "-Wno-" "-W" _flag "${_FLAG}")
    set (CMAKE_REQUIRED_DEFINITIONS "${_flag}")
  endif()

## begin copy [[
  # Normalize locale during test compilation.
  set(_CheckCXXCompilerFlag_LOCALE_VARS LC_ALL LC_MESSAGES LANG)
  foreach(v ${_CheckCXXCompilerFlag_LOCALE_VARS})
    if (DEFINED ENV{${v}})
      set(_CheckCXXCompilerFlag_SAVED_${v} "$ENV{${v}}")
    endif()
    set(ENV{${v}} C)
  endforeach()
  CHECK_COMPILER_FLAG_COMMON_PATTERNS(_CheckCXXCompilerFlag_COMMON_PATTERNS)
  CHECK_CXX_SOURCE_COMPILES("int main() { return 0; }" ${_RESULT}
    # Some compilers do not fail with a bad flag
    FAIL_REGEX "command line option .* is valid for .* but not for C\\\\+\\\\+" # GNU
## end copy ]]
    FAIL_REGEX ".*-Werror=.* argument .*-Werror=.* is not valid for C\\\\+\\\\+"
## begin copy [[
    ${_CheckCXXCompilerFlag_COMMON_PATTERNS}
    )
  foreach(v ${_CheckCXXCompilerFlag_LOCALE_VARS})
    if (DEFINED _CheckCXXCompilerFlag_SAVED_${v})
      set(ENV{${v}} ${${_CheckCXXCompilerFlag_SAVED_${v}}})
      unset(_CheckCXXCompilerFlag_SAVED_${v})
    else()
      unset (ENV{${v}})
    endif()
  endforeach()
  unset(_CheckCXXCompilerFlag_LOCALE_VARS)
  unset(_CheckCXXCompilerFlag_COMMON_PATTERNS)

  set (CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
endmacro ()
## end copy ]]

macro (add_local_compiler_flag_if_supported _flag _variable)
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_flag}" _test_variable)
  check_cxx_compiler_flag ("${_flag}" ${_test_variable})
  if (${_test_variable})
    list (APPEND ${_variable} "${_flag}")
  endif()
endmacro()

macro (add_noggit_compiler_flag_if_supported _flag)
  add_local_compiler_flag_if_supported (${_flag} NOGGIT_CXX_FLAGS)
endmacro()

macro (add_global_compiler_flag_if_supported _flag)
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_flag}" _test_variable)
  check_cxx_compiler_flag ("${_flag}" ${_test_variable})
  if (${_test_variable})
    add_compile_options ("${_flag}")
  endif()
endmacro()

macro (add_global_compiler_flag_if_supported_config _flag) #, _configs...
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_flag}" _test_variable)
  check_cxx_compiler_flag ("${_flag}" ${_test_variable})
  if (${_test_variable})
    foreach (_config IN ITEMS ${ARGN})
      add_compile_options ("$<$<CONFIG:${_config}>:${_flag}>")
    endforeach()
  endif()
endmacro()
