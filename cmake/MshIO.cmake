if (TARGET mshio::mshio)
    return()
endif()

message(STATUS "Third-party (external): creating target 'mshio::mshio'")

include(CPM)
CPMAddPackage(
  NAME mshio
  GITHUB_REPOSITORY qnzhou/MshIO
  GIT_TAG 8d3254b0c4408f914f4074d0f4d9be5d8beff0a3
)

set_target_properties(mshio PROPERTIES FOLDER third_party)
