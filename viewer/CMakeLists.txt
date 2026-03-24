# Viewer CMakeLists.txt

# Main viewer source
set(VIEWER_SOURCES
    main.cc
)

# Framework sources
set(FRAMEWORK_SOURCES
    framework/application.cc
)

set(FRAMEWORK_HEADERS
    framework/application.h
    framework/base/curve_editor.h
)

# Bezier curve implementation
set(BEZIER_SOURCES
    impl/bezier/curve_editor.cc
)

set(BEZIER_HEADERS
    impl/bezier/curve_editor.h
)

# Collect all sources
set(ALL_SOURCES
    ${VIEWER_SOURCES}
    ${FRAMEWORK_SOURCES}
    ${BEZIER_SOURCES}
)

# Collect all headers
set(ALL_HEADERS
    ${FRAMEWORK_HEADERS}
    ${BEZIER_HEADERS}
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
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Compiler warnings
if(MSVC)
    target_compile_options(cagd_viewer PRIVATE /W4)
else()
    target_compile_options(cagd_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()
