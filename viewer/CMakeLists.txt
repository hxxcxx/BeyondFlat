# Viewer CMakeLists.txt

# Main viewer source
set(VIEWER_SOURCES
    main.cc
)

# Framework sources
set(FRAMEWORK_SOURCES
    framework/application.cc
    framework/gl/camera.cc
    framework/gl/renderer.cc
    framework/gl/viewport.cc
)

set(FRAMEWORK_HEADERS
    framework/application.h
    framework/base/curve_editor.h
    framework/gl/camera.h
    framework/gl/renderer.h
    framework/gl/viewport.h
)

# Bezier curve implementation
set(BEZIER_SOURCES
    impl/bezier/curve_editor.cc
)

set(BEZIER_HEADERS
    impl/bezier/curve_editor.h
)

# Bezier surface implementation
set(SURFACE_SOURCES
    impl/bezier/surface_editor.cc
)

set(SURFACE_HEADERS
    impl/bezier/surface_editor.h
)

# B-spline curve implementation
set(BSPLINE_SOURCES
    impl/bspline/curve_editor.cc
)

set(BSPLINE_HEADERS
    impl/bspline/curve_editor.h
)

# NURBS curve implementation
set(NURBS_SOURCES
    impl/nurbs/curve_editor.cc
)

set(NURBS_HEADERS
    impl/nurbs/curve_editor.h
)

# Collect all sources
set(ALL_SOURCES
    ${VIEWER_SOURCES}
    ${FRAMEWORK_SOURCES}
    ${BEZIER_SOURCES}
    ${SURFACE_SOURCES}
    ${BSPLINE_SOURCES}
    ${NURBS_SOURCES}
)

# Collect all headers
set(ALL_HEADERS
    ${FRAMEWORK_HEADERS}
    ${BEZIER_HEADERS}
    ${SURFACE_HEADERS}
    ${BSPLINE_HEADERS}
    ${NURBS_HEADERS}
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
    glad::glad
)

# Set include directories
target_include_directories(cagd_viewer PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Compiler warnings
if(MSVC)
    target_compile_options(cagd_viewer PRIVATE /W4 /FS)
else()
    target_compile_options(cagd_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()
