# Linear algebra library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE linear_sources
    "*.cc"
)

file(GLOB_RECURSE linear_headers
    "*.h"
)

# Create linear algebra library (include headers for IDE)
add_library(linear_lib 
    ${linear_sources}
    ${linear_headers}
)

# Set include directories
target_include_directories(linear_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler warnings
if(MSVC)
    target_compile_options(linear_lib PRIVATE /W4)
else()
    target_compile_options(linear_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()
