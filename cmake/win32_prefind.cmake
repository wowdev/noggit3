
# Set this to more paths you windows guys need.
# this shouldnt be here....
SET(CMAKE_INCLUDE_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/include/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/include/" "C:/Program Files (x86)/SDL-1.2.14/include/SDL/" "C:/Program Files (x86)/glew-1.5.7/include/" "C:/Program Files (x86)/StormLib/src/")
SET(CMAKE_LIBRARY_PATH "D:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/lib/" "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/lib/" "C:/Program Files (x86)/StormLib/bin/" )

SET(Boost_USE_STATIC_LIBS ON)

#do we really need this
SET(Boost_FIND_QUIETLY OFF) 
SET(BOOST_ROOT ${Boost_INCLUDE_DIR})