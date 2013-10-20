SET(Boost_USE_STATIC_LIBS ON)

# If we are on OSX, we need additional files for SDL to work.
MESSAGE( STATUS "Also using osx/SDLMain.m" )
SET( SourceFiles ${SourceFiles} "${CMAKE_SOURCE_DIR}/include/osx/SDLMain.m" )