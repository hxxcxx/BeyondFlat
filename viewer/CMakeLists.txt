# Viewer CMakeLists.txt

# Collect viewer source files
file(GLOB_RECURSE viewer_sources
    "*.cc"
)

file(GLOB_RECURSE viewer_headers
    "*.h"
)

# Collect demo source files
file(GLOB_RECURSE demo_sources
    "demos/*.cc"
)

file(GLOB_RECURSE demo_headers
    "demos/*.h"
)

# Create viewer executable (include headers for IDE)
add_executable(cagd_viewer 
    ${viewer_sources} 
    ${viewer_headers}
    ${demo_sources}
    ${demo_headers}
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
