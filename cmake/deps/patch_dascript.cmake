# please don't spam root source directory with test things
file (READ "CMakeLists.txt" cmakelists_content)
string (REPLACE "UNITIZE_BUILD(\"examples/test/unit_tests\" TEST_GENERATED_SRC)" "# noggit patch: examples removed" cmakelists_content "${cmakelists_content}")
file (WRITE "CMakeLists.txt" "${cmakelists_content}")
