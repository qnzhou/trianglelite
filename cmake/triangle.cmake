include_guard()

if (NOT TARGET triangle::triangle)
    FetchContent_Declare(
        triangle
        GIT_REPOSITORY https://github.com/PyMesh/triangle.git
        GIT_TAG        master
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(triangle)
    target_include_directories(triangle PUBLIC ${triangle_SOURCE_DIR})
    target_compile_definitions(triangle PRIVATE -DANSI_DECLARATORS)
    if (MSVC)
        target_compile_options(triangle PRIVATE /wd4311 /wd4312)
    endif()
    add_library(triangle::triangle ALIAS triangle)
endif()
