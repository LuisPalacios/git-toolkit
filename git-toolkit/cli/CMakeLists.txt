add_executable(gittoolkit_cli main.cpp)

target_link_libraries(gittoolkit_cli PRIVATE gittoolkit_core)

set_target_properties(gittoolkit_cli PROPERTIES
    OUTPUT_NAME "git-toolkit"
)
