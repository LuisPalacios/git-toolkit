add_executable(gittoolkit_gui WIN32
    main.cpp
    MainWindow.cpp
)

target_link_libraries(gittoolkit_gui
    PRIVATE gittoolkit_core comctl32
)

target_compile_definitions(gittoolkit_gui
    PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
)

set_target_properties(gittoolkit_gui PROPERTIES
    OUTPUT_NAME "git-toolkit-gui"
)

# Usar nuestro manifiesto en lugar del auto-generado por CMake
target_sources(gittoolkit_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app.manifest)
