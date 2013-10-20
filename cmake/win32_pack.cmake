install(TARGETS noggit DESTINATION .)
install(DIRECTORY bin/shaders DESTINATION .)
install(DIRECTORY bin/fonts DESTINATION .)
install(FILES bin/noggit_template.conf bin/freetype6.dll bin/SDL.dll bin/StormLib.dll bin/glew32.dll bin/zlib1.dll DESTINATION .)

set(CPACK_GENERATOR "ZIP;NSIS")
SET(CPACK_BUNDLE_NAME "Noggit 3")
SET(CPACK_PACKAGE_FILE_NAME "Noggit_3.${hg_rev_id_str}")
SET(CPACK_PACKAGE_VERSION "SDL 1.3.${hg_rev_id_str}")

include(CPack)