cmake_minimum_required(VERSION 3.15)
project(mvp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
string(REGEX REPLACE "^v" "" GIT_VERSION ${GIT_VERSION})
string(REGEX MATCH "^[0-9.]+(-beta[0-9]*|-rc[0-9]*|-dev[0-9]*)?(-[0-9]+)?" GIT_VERSION_SHORT ${GIT_VERSION})
string(REGEX MATCH "^[0-9.]+" GIT_VERSION_TAG ${GIT_VERSION})

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

message("-- GIT_VERSION=${GIT_VERSION}")
message("-- GIT_VERSION_SHORT=${GIT_VERSION_SHORT}")
message("-- GIT_VERSION_TAG=${GIT_VERSION_TAG}")
message("-- GIT_BRANCH=${GIT_BRANCH}")

add_subdirectory(external)

get_property(LIBMVP_INCLUDES TARGET "libmvp" PROPERTY INCLUDE_DIRECTORIES)
include_directories(${LIBMVP_INCLUDES} "${CMAKE_CURRENT_SOURCE_DIR}/external/libmvp/src")

add_subdirectory(src)

install(FILES "${CMAKE_SOURCE_DIR}/LICENSE.TXT" DESTINATION .)

# CPack configuration used for 'make package'

if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(PACKAGE_ARCH_STRING "x64")
else()
    set(PACKAGE_ARCH_STRING "x32")
endif()

set(CPACK_PACKAGE_VERSION "${GIT_VERSION_SHORT}")
set(CPACK_PACKAGE_VENDOR "mesytec")

if (WIN32)
    option(MVP_CPACK_INSTALL_DEPLOY_FILES
        "Install additional library files from a special deploy directory." OFF)

    set(MVME_CPACK_DEPLOY_DIR "c:/src/mvme2-deploy/windows/mvme-${PACKAGE_ARCH_STRING}-libusb-0.1-Release"
        CACHE PATH "Directory containing additional files to be deployed."
    )

    if (MVP_CPACK_INSTALL_DEPLOY_FILES)
        install(DIRECTORY "${MVME_CPACK_DEPLOY_DIR}/" DESTINATION . USE_SOURCE_PERMISSIONS)
    endif()

    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_SOURCE_GENERATOR "ZIP")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
    set(CPACK_NSIS_PACKAGE_NAME "mvp")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "mvp.exe")
    set(CPACK_NSIS_PACKAGE_NAME "mvme")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "bin\\\\mvme.exe")
    set(CPACK_NSIS_URL_INFO_ABOUT "http://www.mesytec.com")
    set(CPACK_NSIS_CONTACT "info@mesytec.com")
else()
    option(MVP_CPACK_USE_LINUXDEPLOYQT
        "Use 'linuxdeployqt' to create universal binary packages." OFF)

    set(CPACK_SOURCE_GENERATOR "TBZ2")

    if (MVP_CPACK_USE_LINUXDEPLOYQT)
        # Find linuxdeployqt here to error out early in case it's missing.
        find_program(LINUXDEPLOYQT_EXECUTABLE linuxdeployqt REQUIRED)
        set(CPACK_GENERATOR "External")
        set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/CPackExternal_linuxdeployqt.cmake")
        set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
    else()
        set(CPACK_GENERATOR "TBZ2")
    endif()
endif()

# CPACK_PROJECT_CONFIG_FILE
# File included at cpack time, once per generator after setting CPACK_GENERATOR
# to the actual generator being used; allows per-generator setting of CPACK_*
# variables at cpack time.
# Default: ${PROJECT_BINARY_DIR}/CPackOptions.cmake
configure_file("CPackOptions.cmake.in" "${PROJECT_BINARY_DIR}/CPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/CPackOptions.cmake")

include(CPack)

# vim:tw=0
