// CAGD CLI Managed Wrapper - Main Include File
// Exposes C++ CAGD library to C# via C++/CLI

#pragma once

// Common types
#include "cagd_cli_types.h"

// Common utilities
#include "common/glmath_cli.h"
#include "common/utils_cli.h"

// Curve wrappers
#include "bezier/curve_cli.h"
#include "bspline/curve_cli.h"
#include "nurbs/curve_cli.h"

// Surface wrappers
#include "surfaces/surface_cli.h"
