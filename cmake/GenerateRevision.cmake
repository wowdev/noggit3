# This file is part of Noggit3, licensed under GNU General Public License (version 3).

# assumes working directory in git repo
# assumes var _noggit_revision_template_file
# assumes var _noggit_revision_output_file
# assumes var _noggit_revision_state_file
# assumes var GIT_EXECUTABLE
# generates file "${_noggit_revision_state_file}"
# generates file "${_noggit_revision_output_file}"


macro (execute _output_var) # , command...
  execute_process (COMMAND ${ARGN}
    OUTPUT_VARIABLE ${_output_var} OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _result)
  if (NOT _result EQUAL 0)
    message (FATAL_ERROR "'${ARGN}' failed.")
  endif()
endmacro()

execute (_status "${GIT_EXECUTABLE}" "status" "--porcelain")
set (_dirty_marker "+")
if (_status STREQUAL "")
  set (_dirty_marker "")
endif()

# \todo If head is tagged, use tag.
# \todo Add deploy script that adds tags so that we have sane
# revisions again, at all times.
execute (_revision "${GIT_EXECUTABLE}" "rev-parse" "--short" "HEAD")

file (READ "${_noggit_revision_template_file}" _template_blob)
file (READ "${CMAKE_CURRENT_LIST_FILE}" _self_blob)
string (SHA256 _state_hash "${_dirty_marker}${_revision}${_template_blob}${_self_blob}")

if (EXISTS ${_noggit_revision_output_file})
  file (READ "${_noggit_revision_state_file}" _old_state_hash)
  if (_state_hash STREQUAL _old_state_hash)
    return()
  endif()
endif()

file (WRITE "${_noggit_revision_state_file}" "${_state_hash}")

set (NOGGIT_GIT_VERSION_STRING "${_revision}${_dirty_marker}")
configure_file ("${_noggit_revision_template_file}"
                "${_noggit_revision_output_file}" @ONLY)
