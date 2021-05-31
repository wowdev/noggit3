# This file is part of Noggit3, licensed under GNU General Public License (version 3).

# warnings?
if( NOGGIT_ALL_WARNINGS )
  MESSAGE( STATUS "Spilling out mass warnings." )
  set(noggit_warning_flags "${noggit_warning_flags} -W -Wall -Wshadow")
endif( NOGGIT_ALL_WARNINGS )
