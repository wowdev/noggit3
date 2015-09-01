# warnings?
if( NOGGIT_ALL_WARNINGS )
  MESSAGE( STATUS "Spilling out mass warnings." )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wshadow")
endif( NOGGIT_ALL_WARNINGS )