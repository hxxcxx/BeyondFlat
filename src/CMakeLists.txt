# CAGD core library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE cagd_sources
    "*.cc"
)

file(GLOB_RECURSE cagd_headers
    "*.h"
)

# Create CAGD core library (include headers for IDE)
add_library(cagd_core 
    ${cagd_sources}
    ${cagd_headers}
)

# Set include directories
target_include_directories(cagd_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with Eigen from vcpkg
target_link_libraries(cagd_core PUBLIC Eigen3::Eigen)

# Compiler warnings
if(MSVC)
    target_compile_options(cagd_core PRIVATE /W4)
else()
    target_compile_options(cagd_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
