STRING(REGEX REPLACE "/[^/]*$" "" Boost_STRIPPED_LIB_DIR "${Boost_THREAD_LIBRARY_DEBUG}")
LINK_DIRECTORIES(${Boost_STRIPPED_LIB_DIR} ${STORMLIB_LIBRARY_DIR})

#maybe in wrong order now
SET(IncludeDirectories ${IncludeDirectories} "${CMAKE_SOURCE_DIR}/include/win/")
SET(SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/win/StackWalker.cpp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /vmg /D NOMINMAX")

# warnings?
if( NOGGIT_ALL_WARNINGS )
  	MESSAGE( STATUS "Spilling out mass warnings." )
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wall /Wp64")
else(NOGGIT_ALL_WARNINGS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1 /Wall /Wp64")
endif( NOGGIT_ALL_WARNINGS )

# mark 32 bit executables large address aware so they can use > 2GB address space
if(CMAKE_SIZEOF_VOID_P MATCHES 4)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
	message(STATUS "- MSVC: Enabled large address awareness!")
endif()

set(ResFiles media/res.rc)