# This file is part of Noggit3, licensed under GNU General Public License (version 3).

STRING(REGEX REPLACE "/[^/]*$" "" Boost_STRIPPED_LIB_DIR "${Boost_THREAD_LIBRARY_DEBUG}")
LINK_DIRECTORIES(${Boost_STRIPPED_LIB_DIR} ${STORMLIB_LIBRARY_DIR})

#maybe in wrong order now
SET(IncludeDirectories ${IncludeDirectories} "${CMAKE_SOURCE_DIR}/include/win/")
SET(SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/win/StackWalker.cpp")
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /vmg)
add_compile_definitions (NOMINMAX)

# warnings?
if( NOGGIT_ALL_WARNINGS )
    MESSAGE( STATUS "Spilling out mass warnings." )
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /W4)
    add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /Wall)
endif( NOGGIT_ALL_WARNINGS )

# mark 32 bit executables large address aware so they can use > 2GB address space
if(CMAKE_SIZEOF_VOID_P MATCHES 4)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
  message(STATUS "- MSVC: Enabled large address awareness!")
endif()

set(ResFiles media/res.rc)
