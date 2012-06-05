find_package (Lua51 REQUIRED)
find_package (Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

add_executable (lua_brush_test "lua_brush_test.cpp")

include (${QT_USE_FILE})

include_directories (${LUA_INCLUDE_DIR})

target_link_libraries ( lua_brush_test
                        ${QT_LIBRARIES}
                        ${LUA_LIBRARIES}
                      )
