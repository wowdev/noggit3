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

  # Normalize locale during test compilation.
  set(_CheckCXXCompilerFlag_LOCALE_VARS LC_ALL LC_MESSAGES LANG)
  foreach(v ${_CheckCXXCompilerFlag_LOCALE_VARS})
    set(_CheckCXXCompilerFlag_SAVED_${v} "$ENV{${v}}")
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
    set(ENV{${v}} ${_CheckCXXCompilerFlag_SAVED_${v}})
    unset(_CheckCXXCompilerFlag_SAVED_${v})
  endforeach()
  unset(_CheckCXXCompilerFlag_LOCALE_VARS)
  unset(_CheckCXXCompilerFlag_COMMON_PATTERNS)

  set (CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
endmacro ()
## end copy ]]

macro (add_compiler_flag_if_supported _FLAG)
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_FLAG}" _test_variable)
  check_cxx_compiler_flag ("${_FLAG}" ${_test_variable})
  if (${_test_variable})
    if ("${CMAKE_CXX_FLAGS}" STREQUAL "")
      set (CMAKE_CXX_FLAGS "${_FLAG}")
    else()
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_FLAG}")
    endif()
  endif()
endmacro()
