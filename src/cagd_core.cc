#include "cagd_core.h"
#include <cmath>

namespace cagd {

// Placeholder implementation
// TODO: Implement actual linear algebra operations

// Example: Compute distance between two 3D points
double distance(const Point3d& p1, const Point3d& p2) {
    return (p1 - p2).norm();
}

// Example: Compute the midpoint of two points
Point3d midpoint(const Point3d& p1, const Point3d& p2) {
    return (p1 + p2) / 2.0;
}

// Example: Create a rotation matrix around Z-axis
Matrix3d rotationZ(double angle) {
    double c = std::cos(angle);
    double s = std::sin(angle);
    Matrix3d R;
    R << c, -s, 0,
         s,  c, 0,
         0,  0, 1;
    return R;
}

} // namespace cagd
