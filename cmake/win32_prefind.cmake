
# Set this to more paths you windows guys need.
# this shouldnt be here....steff pls try to use the vars *_DIR or LIB_ROOT
SET(CMAKE_INCLUDE_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/include/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/include/" "C:/Program Files (x86)/SDL-1.2.14/include/SDL/" "C:/Program Files (x86)/glew-1.5.7/include/" "C:/Program Files (x86)/StormLib/src/")
SET(CMAKE_LIBRARY_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/lib/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/lib/" "C:/Program Files (x86)/StormLib/bin/" )

if(LIB_ROOT)
	set(FREETYPE_DIR "${LIB_ROOT}/freetype")
	set(SDL_DIR "${LIB_ROOT}/sdl")
	set(STORMLIB_DIR "${LIB_ROOT}/stormlib")
	set(GLEW_DIR "${LIB_ROOT}/glew")
	set(BOOST_ROOT "${LIB_ROOT}/boost")
endif(LIB_ROOT)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${FREETYPE_DIR} ${SDL_DIR} ${STORMLIB_DIR} ${GLEW_DIR})
SET(Boost_USE_STATIC_LIBS ON)