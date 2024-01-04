include_guard()

if (NOT TARGET triangle::triangle)
    FetchContent_Declare(
        triangle
        GIT_REPOSITORY https://github.com/libigl/triangle.git
        GIT_TAG        6bbd92c7ddd6c803c403e005e1132eadb38fbe68
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(triangle)
    target_include_directories(triangle PUBLIC ${triangle_SOURCE_DIR})
    target_compile_definitions(triangle PRIVATE -DANSI_DECLARATORS)
    if (MSVC)
        target_compile_options(triangle PRIVATE
            /wd4311  # Pointer truncation.
            /wd4244  # Flout convert to int will lose data.
            /wd4312  # Assign 32bit int to 64bit int.
            /wd4996  # strcpy is deprecated.
            )
    endif()
    add_library(triangle::triangle ALIAS triangle)
endif()
