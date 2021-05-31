# set includes etc relative to fastnoise2 root directory, not noggit's
file (GLOB_RECURSE cmakelists LIST_DIRECTORIES FALSE "CMakeLists.txt")
foreach (cmakelist ${cmakelists})
  file (READ "${cmakelist}" cmakelists_content)
  string (REPLACE "CMAKE_SOURCE_DIR" "PROJECT_SOURCE_DIR" cmakelists_content "${cmakelists_content}")
  file (WRITE "${cmakelist}" "${cmakelists_content}")
endforeach()
