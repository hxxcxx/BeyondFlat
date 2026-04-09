// CAGD CLI Managed Wrapper - Common utilities
#pragma once

#include "../cagd_cli_types.h"

namespace cagdcli {

    // ========================================================================
    // Common utilities
    // ========================================================================

    public ref class CommonUtils {
    public:
        static double Distance(Vec3 p1, Vec3 p2) { return cagd::distance(p1.ToNative(), p2.ToNative()); }
        static double Distance(Vec2 p1, Vec2 p2) { return cagd::distance(p1.ToNative(), p2.ToNative()); }
        static Vec3 Midpoint(Vec3 p1, Vec3 p2) { return Vec3(cagd::midpoint(p1.ToNative(), p2.ToNative())); }
        static Vec2 Midpoint(Vec2 p1, Vec2 p2) { return Vec2(cagd::midpoint(p1.ToNative(), p2.ToNative())); }
        static double DegToRad(double degrees) { return cagd::degToRad(degrees); }
        static double RadToDeg(double radians) { return cagd::radToDeg(radians); }
        static double Clamp(double value, double min, double max) { return cagd::clamp(value, min, max); }
        static double Lerp(double a, double b, double t) { return cagd::lerp(a, b, t); }
    };

} // namespace cagdcli
