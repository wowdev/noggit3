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

set (hg_rev_hash_str "Archive")
set (hg_rev_hash "0")
set (hg_rev_id_str "0")
set (hg_rev_id "0")

if (HG_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.hg")
  execute_process (COMMAND "${HG_EXECUTABLE}" id -n
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE hg_rev_id_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process (COMMAND "${HG_EXECUTABLE}" id -i
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE hg_rev_hash_str
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
elseif (EXISTS "${CMAKE_SOURCE_DIR}/.hg_archival.txt")
  file (READ "${CMAKE_SOURCE_DIR}/.hg_archival.txt" hg_rev_hash_str
    LIMIT 10
    OFFSET 7
    NEWLINE_CONSUME
  )
  string (STRIP ${hg_rev_hash_str} hg_rev_hash_str)
  set (hg_rev_id_str "Archive")
  set (hg_rev_id "0")
  set (hg_rev_hash ${hg_rev_hash_str})
endif()

if (NOT hg_rev_id_str)
  if (NOT HG_FOUND)
    message (WARNING "Unable to determine revision of source tree: HG was not found (probably not in path).")
  else()
    message (WARNING "Unable to determine revision of source tree. Probably not building from a repository?")
  endif()
endif()

configure_file ("${CMAKE_SOURCE_DIR}/src/helper/repository.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/helper/repository.h"
  @ONLY
)
