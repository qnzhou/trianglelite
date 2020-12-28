# ============================================================================
# This file will generate the cmake target `trianglelite::trianglelite`.
#
# To add trianglelite as a dependency:
#
#   add_subdirectory(trianglelite)
#   target_link_libraries(your_target trianglelite::trianglelite)
#
# ============================================================================

cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 14)

project(trianglelite)

option(TRIANGLELITE_SINGLE "Use float instead of double" OFF)
option(TRIANGLELITE_UNIT_TESTS "Build unit tests" OFF)

include(FetchContent)
include(cmake/Eigen3.cmake)
include(cmake/triangle.cmake)

file(GLOB INC_FILES "${PROJECT_SOURCE_DIR}/include/trianglelite/*.h")
file(GLOB SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(trianglelite STATIC ${SRC_FILES} ${INC_FILES})
target_link_libraries(trianglelite PUBLIC Eigen3::Eigen PRIVATE triangle::triangle)
target_include_directories(trianglelite PUBLIC "${PROJECT_SOURCE_DIR}/include/")
if (TRIANGLELITE_SINGLE)
    target_compile_definitions(trianglelite PUBLIC -DTRIANGLELITE_SINGLE)
endif()

add_library(trianglelite::trianglelite ALIAS trianglelite)

if (TRIANGLELITE_UNIT_TESTS)
    include(CTest)
    enable_testing()
    include(cmake/Catch2.cmake)

    file(GLOB TEST_FILES "${PROJECT_SOURCE_DIR}/tests/*.cpp")
    add_executable(trianglelite_test ${TEST_FILES})
    target_link_libraries(trianglelite_test trianglelite::trianglelite Catch2::Catch2)
    #target_compile_definitions(trianglelite_test PRIVATE -DCATCH_CONFIG_ENABLE_BENCHMARKING)
    catch_discover_tests(trianglelite_test)
endif()
