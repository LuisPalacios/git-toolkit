cmake_minimum_required(VERSION 3.20)
project(git-toolkit
    VERSION 0.1.0
    DESCRIPTION "Multi-account Git management across providers"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies via FetchContent
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json Catch2)

# Enable testing at top level so ctest works
enable_testing()

# Core library
add_subdirectory(core)

# CLI frontend
add_subdirectory(cli)

# Platform-native GUI
# if(APPLE)
#     add_subdirectory(gui/macos)
# elseif(WIN32)
#     add_subdirectory(gui/windows)
# elseif(UNIX)
#     add_subdirectory(gui/linux)
# endif()
