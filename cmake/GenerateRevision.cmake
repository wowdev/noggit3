# This file is part of Noggit3, licensed under GNU General Public License (version 3).

# Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

find_package (Hg QUIET)
find_package (Git QUIET)

set (hg_rev_hash_str "Archive")
set (hg_rev_hash "0")
set (hg_rev_id_str "0")
set (hg_rev_id "0")

if (HG_FOUND AND EXISTS "${cmake_src_dir}/.hg")
  execute_process (COMMAND "${HG_EXECUTABLE}" id -n
    WORKING_DIRECTORY "${cmake_src_dir}"
    OUTPUT_VARIABLE hg_rev_id_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process (COMMAND "${HG_EXECUTABLE}" id -i
    WORKING_DIRECTORY "${cmake_src_dir}"
    OUTPUT_VARIABLE hg_rev_hash_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
elseif (EXISTS "${cmake_src_dir}/.hg_archival.txt")
  file (READ "${cmake_src_dir}/.hg_archival.txt" hg_rev_hash_str
    LIMIT 10
    OFFSET 7
    NEWLINE_CONSUME
  )
  string (STRIP ${hg_rev_hash_str} hg_rev_hash_str)
  set (hg_rev_id_str "Archive")
  set (hg_rev_id "0")
  set (hg_rev_hash ${hg_rev_hash_str})
elseif (GIT_FOUND AND HG_FOUND AND EXISTS "${cmake_src_dir}/.git/hg")
  execute_process (COMMAND "${HG_EXECUTABLE}" -R "${cmake_src_dir}/.git/hg" tags
                   COMMAND grep ^tip
                   COMMAND sed -e s,tip[[:space:]]*,, -e s,:.*,,
    WORKING_DIRECTORY "${cmake_src_dir}"
    OUTPUT_VARIABLE hg_rev_id_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process (COMMAND "${HG_EXECUTABLE}" -R "${cmake_src_dir}/.git/hg" tags
                   COMMAND grep ^tip
                   COMMAND sed -e s,tip[[:space:]]*,, -e s,.*:,,
    WORKING_DIRECTORY "${cmake_src_dir}"
    OUTPUT_VARIABLE hg_rev_hash_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process (COMMAND "${GIT_EXECUTABLE}" diff --name-only
    WORKING_DIRECTORY "${cmake_src_dir}"
    OUTPUT_VARIABLE git_diff_files
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )
  if (NOT "${git_diff_files}" STREQUAL "")
    set (hg_rev_id_str "${hg_rev_id_str}+")
  endif()
endif()

if (NOT hg_rev_id_str)
  if (NOT HG_FOUND)
    message (WARNING "Unable to determine revision of source tree: HG was not found (probably not in path).")
  else()
    message (WARNING "Unable to determine revision of source tree. Probably not building from a repository?")
  endif()
endif()

# load the previous version str
set(previous_rev_file "${out_dir}/tmp/previous_rev")
file(TOUCH ${previous_rev_file})
file(READ ${previous_rev_file} previous_hg_rev)

if(NOT "${hg_rev_id_str}" STREQUAL "${previous_hg_rev}")
  configure_file ("${cmake_src_dir}/cmake/revision.h.in.cmake"
    "${out_dir}/tmp/revision.h"
    @ONLY
  )
  
  #update the stored version
  file(WRITE ${previous_rev_file} "${hg_rev_id_str}")
endif()
