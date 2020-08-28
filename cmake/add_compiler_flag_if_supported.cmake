# This file is part of Noggit3, licensed under GNU General Public License (version 3).

include_guard (GLOBAL)

include (CheckCXXCompilerFlag)

macro (add_compiler_flag_if_supported _VAR _FLAG)
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_FLAG}" _test_variable)
  check_cxx_compiler_flag ("${_FLAG}" ${_test_variable})
  if (${_test_variable})
    if ("${${_VAR}}" STREQUAL "")
      set (${_VAR} "${_FLAG}")
    else()
      set (${_VAR} "${${_VAR}} ${_FLAG}")
    endif()
  endif()
endmacro()
