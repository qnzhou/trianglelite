if (TARGET fmt::fmt)
    return()
endif()

message(STATUS "Third-party (external): creating target 'fmt::fmt'")

include(CPM)
CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 10.2.1
)

set_target_properties(fmt PROPERTIES FOLDER third_party)
