if (TARGET triangle::triangle)
    return()
endif()

message(STATUS "Third-party (external): creating target 'triangle::triangle'")

include(CPM)
CPMAddPackage(
    NAME triangle
    GITHUB_REPOSITORY libigl/triangle
    GIT_TAG           6bbd92c7ddd6c803c403e005e1132eadb38fbe68
)

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
