set(CORE_SOURCES
    src/log.cpp
    src/platform.cpp
    src/config.cpp
    src/git_ops.cpp
    src/sync_status.cpp
    src/engine.cpp
)

set(CORE_HEADERS
    include/gittoolkit/log.h
    include/gittoolkit/platform.h
    include/gittoolkit/config.h
    include/gittoolkit/git_ops.h
    include/gittoolkit/sync_status.h
    include/gittoolkit/engine.h
)

add_library(gittoolkit_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(gittoolkit_core
    PUBLIC include
)

target_link_libraries(gittoolkit_core
    PUBLIC nlohmann_json::nlohmann_json
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(gittoolkit_core PRIVATE advapi32 credui)
elseif(APPLE)
    find_library(SECURITY_FRAMEWORK Security)
    target_link_libraries(gittoolkit_core PRIVATE ${SECURITY_FRAMEWORK})
elseif(UNIX)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBSECRET libsecret-1)
        if(LIBSECRET_FOUND)
            target_include_directories(gittoolkit_core PRIVATE ${LIBSECRET_INCLUDE_DIRS})
            target_link_libraries(gittoolkit_core PRIVATE ${LIBSECRET_LIBRARIES})
        endif()
    endif()
endif()

# Suppress MSVC getenv deprecation warnings
if(WIN32)
    target_compile_definitions(gittoolkit_core PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Tests
add_subdirectory(tests)
