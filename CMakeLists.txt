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
option(TRIANGLELITE_MSHIO "Enable debug msh save" OFF)
option(TRIANGLELITE_PYTHON "Build python bindings" OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(Eigen3)
include(triangle)
include(sanitizer-cmake)

file(GLOB INC_FILES "${PROJECT_SOURCE_DIR}/include/trianglelite/*.h")
file(GLOB SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(trianglelite STATIC ${SRC_FILES} ${INC_FILES})
target_link_libraries(trianglelite PUBLIC Eigen3::Eigen PRIVATE triangle::triangle)
target_include_directories(trianglelite PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_compile_definitions(trianglelite PRIVATE -DEIGEN_NO_MALLOC)
set_target_properties(trianglelite PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (TRIANGLELITE_SINGLE)
    target_compile_definitions(trianglelite PUBLIC -DTRIANGLELITE_SINGLE)
    target_compile_definitions(triangle PUBLIC -DSINGLE)
endif()
if (TRIANGLELITE_MSHIO)
    include(MshIO)
    target_compile_definitions(trianglelite PRIVATE -DWITH_MSHIO)
    target_link_libraries(trianglelite PRIVATE mshio::mshio)
endif()
if (MSVC)
    target_compile_definitions(trianglelite PRIVATE -D_USE_MATH_DEFINES)
endif()

add_library(trianglelite::trianglelite ALIAS trianglelite)

if (TRIANGLELITE_UNIT_TESTS)
    include(CTest)
    enable_testing()
    include(Catch2)

    file(GLOB TEST_FILES "${PROJECT_SOURCE_DIR}/tests/*.cpp")
    add_executable(trianglelite_test ${TEST_FILES})
    target_link_libraries(trianglelite_test trianglelite::trianglelite Catch2::Catch2WithMain)
    catch_discover_tests(trianglelite_test)
endif()

if (SANITIZE_ADDRESS OR
        SANITIZE_LINK_STATIC OR
        SANITIZE_MEMORY OR
        SANITIZE_THREAD OR
        SANITIZE_UNDEFINED)
    add_sanitizers(trianglelite)
    if (TRIANGLELITE_UNIT_TESTS)
        add_sanitizers(trianglelite_test)
    endif()
endif()

if (TRIANGLELITE_PYTHON)
    if(SKBUILD)
        if(APPLE)
            set(CMAKE_INSTALL_RPATH @loader_path)
        elseif(UNIX)
            set(CMAKE_INSTALL_RPATH $ORIGIN)
        endif()
    endif()

    include(nanobind)
    include(fmt)
    set(PY_SRC_FILE "${PROJECT_SOURCE_DIR}/python/pytrianglelite.cpp")
    nanobind_add_module(pytrianglelite NB_STATIC ${PY_SRC_FILE})
    target_compile_features(pytrianglelite PRIVATE cxx_std_17)
    target_link_libraries(pytrianglelite PUBLIC trianglelite::trianglelite fmt::fmt)

    if (SKBUILD)
        nanobind_add_stub(pytrianglelite_stub
            MODULE pytrianglelite
            OUTPUT ${SKBUILD_PLATLIB_DIR}/trianglelite/pytrianglelite.pyi
            DEPENDS pytrianglelite
            COMPONENT TriangleLite_Python_Runtime
            PYTHON_PATH $<TARGET_FILE_DIR:pytrianglelite>
            MARKER_FILE py.typed
            VERBOSE
        )
    endif()

    install(TARGETS pytrianglelite LIBRARY
        COMPONENT TriangleLite_Python_Runtime
        DESTINATION ${SKBUILD_PLATLIB_DIR}/trianglelite)
endif()

