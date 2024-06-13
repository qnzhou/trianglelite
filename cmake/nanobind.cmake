if (COMMAND nanobind_add_module)
    return()
endif()

include(CPM)
include(cmake/python.cmake)
CPMAddPackage(
    NAME nanobind
    GITHUB_REPOSITORY wjakob/nanobind
    GIT_TAG v2.0.0
    GIT_SHALLOW OFF)

find_package(nanobind PATHS ${nanobind_SOURCE_DIR}/cmake NO_DEFAULT_PATH)
