if (COMMAND add_sanitizers)
    return()
endif()

include(CPM)
CPMAddPackage(
    NAME sanitizer
    GITHUB_REPOSITORY arsenm/sanitizers-cmake
    GIT_TAG        3f0542e4e034aab417c51b2b22c94f83355dee15
    DOWNLOAD_ONLY ON
)

set(CMAKE_MODULE_PATH
    "${sanitizer_SOURCE_DIR}/cmake"
    ${CMAKE_MODULE_PATH})
find_package(Sanitizers REQUIRED)
