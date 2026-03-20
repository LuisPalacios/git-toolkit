add_executable(gittoolkit_tests
    test_log.cpp
    test_platform.cpp
)

target_link_libraries(gittoolkit_tests
    PRIVATE gittoolkit_core Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(gittoolkit_tests)
