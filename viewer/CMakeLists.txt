# Viewer CMakeLists.txt

# Main viewer source
set(VIEWER_SOURCES
    main.cc
)

# Application sources
set(APP_SOURCES
    app/application.cc
)

set(APP_HEADERS
    app/application.h
)

# Base classes
set(BASE_SOURCES
    # No base sources yet, only headers
)

set(BASE_HEADERS
    base/curve_editor.h
    base/curve_renderer.h
)

# Editor sources
set(EDITOR_SOURCES
    editors/bezier_editor.cc
)

set(EDITOR_HEADERS
    editors/bezier_editor.h
)

# Renderer sources
set(RENDER_SOURCES
    render/bezier_renderer.cc
)

set(RENDER_HEADERS
    render/bezier_renderer.h
)

# Collect all sources
set(ALL_SOURCES
    ${VIEWER_SOURCES}
    ${APP_SOURCES}
    ${EDITOR_SOURCES}
    ${RENDER_SOURCES}
)

# Collect all headers
set(ALL_HEADERS
    ${APP_HEADERS}
    ${BASE_HEADERS}
    ${EDITOR_HEADERS}
    ${RENDER_HEADERS}
)

# Create viewer executable
add_executable(cagd_viewer
    ${ALL_SOURCES}
    ${ALL_HEADERS}
)

# Link with libraries from vcpkg
target_link_libraries(cagd_viewer PRIVATE
    cagd_core
    imgui::imgui
    glfw
    OpenGL::GL
)

# Set include directories
target_include_directories(cagd_viewer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Compiler warnings
if(MSVC)
    target_compile_options(cagd_viewer PRIVATE /W4)
else()
    target_compile_options(cagd_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()
