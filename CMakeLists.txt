cmake_minimum_required(VERSION 2.8.9)
cmake_policy(SET CMP0020 NEW) # Automatically link Qt executables to qtmain target on Windows.
project(mvp)

## Create binaries in the root of the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add the local cmake directory module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# RPATH handling
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH FALSE)

# Enable $ORIGIN in the rpath if supported by the target platform.
SET(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

# Pull information from git:
# GIT_SHA1 contains the complete git hash
# GIT_VERSION contains the name of the latest git tag and the start of the git hash if
# there have been changes since the tag has been created.
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_describe(GIT_VERSION "--always")
string(REGEX MATCH "^[0-9.]+(-beta|-rc[0-9]*)?(-[0-9]+)?" GIT_VERSION_SHORT ${GIT_VERSION})

add_subdirectory(external)

get_property(LIBMVP_INCLUDES TARGET "libmvp" PROPERTY INCLUDE_DIRECTORIES)
include_directories(${LIBMVP_INCLUDES} "${CMAKE_CURRENT_SOURCE_DIR}/external/libmvp/src")

add_subdirectory(src)

install(FILES "${CMAKE_SOURCE_DIR}/LICENSE.TXT" DESTINATION .)

if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(PACKAGE_ARCH_STRING "x64")
else()
    set(PACKAGE_ARCH_STRING "x32")
endif()

# CPack configuration used for 'make package'
if (WIN32)
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${GIT_VERSION}")

  # Reusing the deploy directory from mvme
  set(DEPLOY_DIR "c:/src/mvme2-deploy/windows/mvme-${PACKAGE_ARCH_STRING}-libusb-0.1-${CMAKE_BUILD_TYPE}/")
  if (EXISTS "${DEPLOY_DIR}")
      install(DIRECTORY "${DEPLOY_DIR}" DESTINATION . USE_SOURCE_PERMISSIONS)
  endif()
else(WIN32)
  execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE)
  set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${GIT_VERSION}-${ARCHITECTURE}")

  # Reusing the deploy directory from mvme
  if (EXISTS "$ENV{HOME}/src/mvme2-deploy/linux/mvme-${PACKAGE_ARCH_STRING}-libusb-0.1/")
      # Install additional deployment files: shared objects, qt plugins, etc.
      install(DIRECTORY "$ENV{HOME}/src/mvme2-deploy/linux/mvme-${PACKAGE_ARCH_STRING}-libusb-0.1/"
          DESTINATION .
          USE_SOURCE_PERMISSIONS)
  endif()
endif(WIN32)

set(CPACK_PACKAGE_VERSION "${GIT_VERSION_SHORT}")
set(CPACK_PACKAGE_VENDOR "mesytec")
set(CPACK_PACKAGE_EXECUTABLES "mvp;mvp")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "mvp")
set(CPACK_OUTPUT_FILE_PREFIX "packages")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/external/libmvp/src/resources/gpl-notice.txt")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_MONOLITHIC_INSTALL TRUE)

if (WIN32)
    set(CPACK_GENERATOR "ZIP" "NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
    set(CPACK_NSIS_PACKAGE_NAME "mvp")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "mvp.exe")
    set(CPACK_NSIS_URL_INFO_ABOUT "http://www.mesytec.com")
    set(CPACK_NSIS_CONTACT "info@mesytec.com")
    set(CPACK_SOURCE_GENERATOR "ZIP")
else(WIN32)
    set(CPACK_GENERATOR "TBZ2")
    set(CPACK_SOURCE_GENERATOR "TBZ2")
endif(WIN32)

include(CPack)

# vim:tw=0
