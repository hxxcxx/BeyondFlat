# CAGD core library CMakeLists.txt

# Common utility sources
set(COMMON_SOURCES
    common/common.cc
)

set(COMMON_HEADERS
    cagd_types.h
    common/common.h
)

# Bezier curve sources
set(BEZIER_SOURCES
    bezier/bezier_curve.cc
)

set(BEZIER_HEADERS
    bezier/bezier_curve.h
)

# Collect all sources
set(CAGD_SOURCES
    ${COMMON_SOURCES}
    ${BEZIER_SOURCES}
)

# Collect all headers
set(CAGD_HEADERS
    ${COMMON_HEADERS}
    ${BEZIER_HEADERS}
)

# Create CAGD core library
add_library(cagd_core
    ${CAGD_SOURCES}
    ${CAGD_HEADERS}
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
