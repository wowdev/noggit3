cmake_minimum_required(VERSION 2.8)

project(Noggit)

#set this to more paths you windows guys need.
set(CMAKE_INCLUDE_PATH "C:/Program Files/Microsoft Visual Studio 9.0/VC/include/" "C:/Program Files/Microsoft SDKs/Windows/v6.0A/Include/")
set(CMAKE_LIBRARY_PATH "C:/Program Files/Microsoft Visual Studio 9.0/VC/lib/" "C:/Program Files/Microsoft SDKs/Windows/v6.0A/lib/")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

find_package(zlib REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(StormLib REQUIRED)
find_package(SDL REQUIRED)

#hack as at least my FindOpenGL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenGL DEFAULT_MSG OPENGL_LIBRARIES OPENGL_INCLUDE_DIR)
#hack as at least my FindSDL does not do this itself.
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL DEFAULT_MSG SDL_LIBRARY SDL_INCLUDE_DIR)

set(INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR} ${STORMLIB_INCLUDES})

INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES(stdint.h HAS_STDINT_H)

#if(!HAS_STDINT_H)
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/win/)
#endif(!HAS_STDINT_H)

file(GLOB_RECURSE HINCS "Src/*.h")
file(GLOB_RECURSE CPPINCS "Src/*.cpp")

set(srcs
${HINCS}
${CPPINCS}
)

IF(APPLE)
    message(STATUS "Also using osx/SDLmain.m")
    SET(srcs ${srcs} "${CMAKE_SOURCE_DIR}/include/osx/SDLmain.m")

    message(STATUS "And writing log to console instead of log.txt")
    add_definitions(-DDEBUG__LOGGINGTOCONSOLE)
ENDIF(APPLE)

include_directories(${INCLUDE_DIRS})

add_executable(noggit ${srcs})

target_link_libraries(noggit ${ZLIB_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL_LIBRARY} ${STORMLIB_LIBRARIES} ${GLEW_LIBRARY})

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noggit DESTINATION ${CMAKE_SOURCE_DIR}/bin/)