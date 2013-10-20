include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "ZIP;NSIS")
SET(CPACK_BUNDLE_NAME "Noggit 3")
SET(CPACK_PACKAGE_FILE_NAME "Noggit 3")
SET(CPACK_PACKAGE_VERSION "SDL 1.3")


include(CPack)