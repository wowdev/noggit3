# This file is part of Noggit3, licensed under GNU General Public License (version 3).

cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name
project(Noggit)


include (CheckCXXCompilerFlag)
macro (add_compiler_flag_if_supported _VAR _FLAG)
  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_FLAG}" _test_variable)
  check_cxx_compiler_flag ("${_FLAG}" ${_test_variable})
  if (${_test_variable})
     set (${_VAR} "${${_VAR}} ${_FLAG}")
  endif()
endmacro()

add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -fcolor-diagnostics)

# covered by CMAKE_CXX_STANDARD
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-c++98-compat)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-c++98-compat-pedantic)

# covered by compilers used
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-gnu-anonymous-struct)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-variadic-macros)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-vla)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-vla-extension)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-zero-length-array)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-gnu-zero-variadic-macro-arguments)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-nested-anon-types)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-four-char-constants)

# we assume that our constructors and destructors do not access other global state
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-exit-time-destructors)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-global-constructors)

# is fine with GNU, required due to our libstdc
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-disabled-macro-expansion)

# we can live with the compilation unit containing the vtable not being fixed
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-weak-vtables)
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-weak-template-vtables)

# __DATE__ and __TIME__ not being reproducible is exactly why they exist.
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-date-time)

# we don't care for a few bytes
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-padded)

# msvc++ mangles struct/class into name, thus symbols may be called differently
# with a bad forward-decl. we want compilation to fail, not linking.
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS /we4099)

# yes, we intend to use multi-character character constants
add_compiler_flag_if_supported (CMAKE_CXX_FLAGS -Wno-multichar)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBARY_OUTPUT_PATH bin)

macro(includePlattform SUFFIX)
  if(UNIX)
    if(APPLE)
      include("${CMAKE_SOURCE_DIR}/cmake/apple_${SUFFIX}.cmake")
    else(APPLE)
      include("${CMAKE_SOURCE_DIR}/cmake/linux_${SUFFIX}.cmake")
    endif(APPLE)
  else(UNIX)
    if(WIN32)
      include("${CMAKE_SOURCE_DIR}/cmake/win32_${SUFFIX}.cmake")
      # adds for library repo
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/lib/")
      # freetype2
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/freetype2/lib/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/freetype2/include/freetype2/" ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/freetype2/include/")
      #glew
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Glew/lib/Release/Win32/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Glew/include/")
      #storm lib
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/DebugAD/" "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/ReleaseAD/" "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/Release/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/StormLib/include/")
      #SDL
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/SDL/lib/x86/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/SDL/include/")
      #boost
      include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
      set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/libs/")
      set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/../Noggit3libs/Boost/")
	  endif(WIN32)
  endif(UNIX)
endmacro(includePlattform)

#find all src files
file(GLOB SourceFiles "src/*/*.h" "src/*/*.hpp" "src/*/*.cpp" "src/*/*.mm" "src/external/wacom/*.cpp" "src/noggit/ui/*.cpp")

includePlattform("prefind")

FIND_PACKAGE( Freetype REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( Boost COMPONENTS thread filesystem system unit_test_framework test_exec_monitor REQUIRED )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( StormLib REQUIRED )
FIND_PACKAGE( SDL REQUIRED )
find_library(MYSQL_LIBRARY
    NAMES libmysql
    HINTS "${CMAKE_SOURCE_DIR}/src/external/mysql"
)
find_library(MYSQLCPPCONN_LIBRARY
    NAMES mysqlcppconn
    HINTS "${CMAKE_SOURCE_DIR}/src/external/mysql/connector"
)


# Hack as at least my FindSDL does not do this itself.
MARK_AS_ADVANCED( SDL_LIBRARY SDL_INCLUDE_DIR )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SDL DEFAULT_MSG SDL_LIBRARY SDL_INCLUDE_DIR )

# Add the found include directories to our include list.
include_directories (SYSTEM ${GLEW_INCLUDE_PATH} ${GLUT_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR})
include_directories (SYSTEM "${CMAKE_SOURCE_DIR}/include/")

OPTION(NOGGIT_ALL_WARNINGS "Enable all warnings?" OFF)

# Log to console for easier debugging.
OPTION( NOGGIT_LOGTOCONSOLE "Log to console instead of log.txt?" OFF )
IF( NOGGIT_LOGTOCONSOLE )
  MESSAGE( STATUS "And writing log to console instead of log.txt" )
  ADD_DEFINITIONS( -DDEBUG__LOGGINGTOCONSOLE )
ENDIF( NOGGIT_LOGTOCONSOLE )

includePlattform("postfind")

# Find revision ID and hash of the sourcetree
include( "${CMAKE_SOURCE_DIR}/cmake/GenerateRevision.cmake" )
include_directories ("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/tmp")

# And do the job.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src" )
ADD_EXECUTABLE( noggit WIN32 MACOSX_BUNDLE ${SourceFiles} ${ResFiles})
TARGET_LINK_LIBRARIES (noggit
  ${OPENGL_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${SDL_LIBRARY}
  StormLib
  Boost::thread
  Boost::filesystem
  Boost::system
  ${GLEW_LIBRARY}
  ${MYSQL_LIBRARY}
  ${MYSQLCPPCONN_LIBRARY}

)

if (NOGGIT_LOGTOCONSOLE AND WIN32)
  set_property (TARGET noggit APPEND PROPERTY LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  set_property (TARGET noggit APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:"_CONSOLE">)
endif()

includePlattform("pack")

add_library (noggit-math STATIC
  "src/math/matrix_4x4.cpp"
  "src/math/vector_2d.cpp"
)
add_library (noggit::math ALIAS noggit-math)

include (CTest)
enable_testing()

add_executable (math-vector_2d.test test/math/vector_2d.cpp)
target_compile_definitions (math-vector_2d.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-vector_2d.test Boost::unit_test_framework Boost::test_exec_monitor noggit::math)
add_test (NAME math-vector_2d COMMAND $<TARGET_FILE:math-vector_2d.test>)

add_executable (math-trig.test test/math/trig.cpp)
target_compile_definitions (math-trig.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-trig.test Boost::unit_test_framework Boost::test_exec_monitor noggit::math)
add_test (NAME math-trig COMMAND $<TARGET_FILE:math-trig.test>)

add_executable (math-matrix_4x4.test test/math/matrix_4x4.cpp)
target_compile_definitions (math-matrix_4x4.test PRIVATE "-DBOOST_TEST_MODULE=\"math\"")
target_link_libraries (math-matrix_4x4.test Boost::unit_test_framework Boost::test_exec_monitor noggit::math)
add_test (NAME math-matrix_4x4 COMMAND $<TARGET_FILE:math-matrix_4x4.test>)
