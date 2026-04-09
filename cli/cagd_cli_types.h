// CAGD CLI Managed Wrapper - Common Types
#pragma once

#pragma managed(push, off)

#include <vector>
#include <utility>
#include <stdexcept>

#include "../src/cagd_types.h"
#include "../src/bezier/bezier_curve.h"
#include "../src/bezier/bezier_surface.h"
#include "../src/bspline/bspline_curve.h"
#include "../src/nurbs/nurbs_curve.h"

#pragma managed(pop)

namespace cagdcli {
    using namespace System;
    using namespace System::Collections::Generic;

    // ========================================================================
    // Vector2d and Vector3d wrappers
    // ========================================================================

    public value struct Vec2 {
        double X, Y;
        Vec2(double x = 0, double y = 0) : X(x), Y(y) {}
        Vec2(const Eigen::Vector2d& v) : X(v.x()), Y(v.y()) {}
        Eigen::Vector2d ToNative() const { return Eigen::Vector2d(X, Y); }
    };

    public value struct Vec3 {
        double X, Y, Z;
        Vec3(double x = 0, double y = 0, double z = 0) : X(x), Y(y), Z(z) {}
        Vec3(const Eigen::Vector3d& v) : X(v.x()), Y(v.y()), Z(v.z()) {}
        Eigen::Vector3d ToNative() const { return Eigen::Vector3d(X, Y, Z); }
    };

} // namespace cagdcli
