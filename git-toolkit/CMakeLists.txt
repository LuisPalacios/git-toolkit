cmake_minimum_required(VERSION 3.20)
project(git-toolkit
    VERSION 0.1.0
    DESCRIPTION "Multi-account Git management across providers"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Core library
# add_subdirectory(core)

# CLI frontend
# add_subdirectory(cli)

# Platform-native GUI
# if(APPLE)
#     add_subdirectory(gui/macos)
# elseif(WIN32)
#     add_subdirectory(gui/windows)
# elseif(UNIX)
#     add_subdirectory(gui/linux)
# endif()
