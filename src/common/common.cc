#include "common.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cagd {

// ============================================================================
// Basic Geometric Operations
// ============================================================================

double distance(const Point3d& p1, const Point3d& p2) {
    return (p1 - p2).norm();
}

double distance(const Point2d& p1, const Point2d& p2) {
    return (p1 - p2).norm();
}

Point3d midpoint(const Point3d& p1, const Point3d& p2) {
    return (p1 + p2) / 2.0;
}

Point2d midpoint(const Point2d& p1, const Point2d& p2) {
    return (p1 + p2) / 2.0;
}

// ============================================================================
// Matrix Transformations
// ============================================================================

Matrix3d rotationX(double angle) {
    double c = std::cos(angle);
    double s = std::sin(angle);
    Matrix3d R;
    R << 1, 0,  0,
         0, c, -s,
         0, s,  c;
    return R;
}

Matrix3d rotationY(double angle) {
    double c = std::cos(angle);
    double s = std::sin(angle);
    Matrix3d R;
    R << c,  0, s,
         0,  1, 0,
        -s,  0, c;
    return R;
}

Matrix3d rotationZ(double angle) {
    double c = std::cos(angle);
    double s = std::sin(angle);
    Matrix3d R;
    R << c, -s, 0,
         s,  c, 0,
         0,  0, 1;
    return R;
}

Matrix4d translation(const Vector3d& offset) {
    Matrix4d T = Matrix4d::Identity();
    T.block<3, 1>(0, 3) = offset;
    return T;
}

Matrix4d scale(double sx, double sy, double sz) {
    Matrix4d S = Matrix4d::Identity();
    S(0, 0) = sx;
    S(1, 1) = sy;
    S(2, 2) = sz;
    return S;
}

// ============================================================================
// Mathematical Utilities
// ============================================================================

double degToRad(double degrees) {
    return degrees * M_PI / 180.0;
}

double radToDeg(double radians) {
    return radians * 180.0 / M_PI;
}

double clamp(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

double lerp(double a, double b, double t) {
    return a + t * (b - a);
}

} // namespace cagd
