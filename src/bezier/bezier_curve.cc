#include "bezier_curve.h"
#include <cmath>
#include <algorithm>

namespace cagd {

// ============================================================================
// BezierCurve2d Implementation
// ============================================================================

BezierCurve2d::BezierCurve2d(const PointVector2d& controlPoints)
    : control_points_(controlPoints) {
    if (control_points_.empty()) {
        throw std::invalid_argument("Control points cannot be empty");
    }
}

Point2d BezierCurve2d::evaluate(double t) const {
    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    const int n = degree();
    Point2d point(0.0, 0.0);

    // Bezier curve formula: B(t) = sum(B_{i,n}(t) * P_i)
    for (int i = 0; i <= n; ++i) {
        double bernstein = bernsteinPolynomial(i, n, t);
        point += bernstein * control_points_[i];
    }

    return point;
}

Point2d BezierCurve2d::deCasteljau(double t) const {
    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Create a working copy of control points
    PointVector2d points = control_points_;
    const int n = degree();

    // De Casteljau algorithm
    for (int k = 1; k <= n; ++k) {
        for (int i = 0; i <= n - k; ++i) {
            points[i] = (1.0 - t) * points[i] + t * points[i + 1];
        }
    }

    return points[0];
}

Point2d BezierCurve2d::derivative(double t, int order) const {
    if (order < 1 || order > degree()) {
        throw std::invalid_argument("Invalid derivative order");
    }

    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Compute control points of derivative curve
    PointVector2d derivPoints;
    const int n = degree();

    for (int i = 0; i < n; ++i) {
        derivPoints.push_back(n * (control_points_[i + 1] - control_points_[i]));
    }

    // Create derivative curve and evaluate
    BezierCurve2d derivCurve(derivPoints);
    return derivCurve.evaluate(t);
}

std::pair<BezierCurve2d, BezierCurve2d> BezierCurve2d::subdivide(double t) const {
    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    const int n = degree();

    // Create a 2D array to store de Casteljau intermediate points
    std::vector<std::vector<Point2d>> pyramid(n + 1);

    // Initialize first level with original control points
    pyramid[0] = control_points_;

    // De Casteljau algorithm - build the pyramid
    for (int k = 1; k <= n; ++k) {
        pyramid[k].resize(n + 1 - k);
        for (int i = 0; i <= n - k; ++i) {
            pyramid[k][i] = (1.0 - t) * pyramid[k - 1][i] + t * pyramid[k - 1][i + 1];
        }
    }

    // Extract control points for left curve [0, t]
    // Left curve control points are the first point of each level
    PointVector2d leftPoints;
    for (int k = 0; k <= n; ++k) {
        leftPoints.push_back(pyramid[k][0]);
    }

    // Extract control points for right curve [t, 1]
    // Right curve control points are the last point of each level (in reverse order)
    PointVector2d rightPoints;
    for (int k = 0; k <= n; ++k) {
        rightPoints.push_back(pyramid[n - k][k]);
    }

    return std::make_pair(BezierCurve2d(leftPoints), BezierCurve2d(rightPoints));
}

// ============================================================================
// BezierCurve3d Implementation
// ============================================================================

BezierCurve3d::BezierCurve3d(const PointVector3d& controlPoints)
    : control_points_(controlPoints) {
    if (control_points_.empty()) {
        throw std::invalid_argument("Control points cannot be empty");
    }
}

Point3d BezierCurve3d::evaluate(double t) const {
    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    const int n = degree();
    Point3d point(0.0, 0.0, 0.0);

    // Bezier curve formula: B(t) = sum(B_{i,n}(t) * P_i)
    for (int i = 0; i <= n; ++i) {
        double bernstein = bernsteinPolynomial(i, n, t);
        point += bernstein * control_points_[i];
    }

    return point;
}

Point3d BezierCurve3d::deCasteljau(double t) const {
    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Create a working copy of control points
    PointVector3d points = control_points_;
    const int n = degree();

    // De Casteljau algorithm
    for (int k = 1; k <= n; ++k) {
        for (int i = 0; i <= n - k; ++i) {
            points[i] = (1.0 - t) * points[i] + t * points[i + 1];
        }
    }

    return points[0];
}

Point3d BezierCurve3d::derivative(double t, int order) const {
    if (order < 1 || order > degree()) {
        throw std::invalid_argument("Invalid derivative order");
    }

    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Compute control points of derivative curve
    PointVector3d derivPoints;
    const int n = degree();

    for (int i = 0; i < n; ++i) {
        derivPoints.push_back(n * (control_points_[i + 1] - control_points_[i]));
    }

    // Create derivative curve and evaluate
    BezierCurve3d derivCurve(derivPoints);
    return derivCurve.evaluate(t);
}

// ============================================================================
// Utility Functions
// ============================================================================

int binomialCoefficient(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;

    // Use symmetry property
    k = std::min(k, n - k);

    int result = 1;
    for (int i = 1; i <= k; ++i) {
        result = result * (n - k + i) / i;
    }

    return result;
}

double bernsteinPolynomial(int i, int n, double t) {
    if (i < 0 || i > n) return 0.0;

    // B_{i,n}(t) = C(n,i) * t^i * (1-t)^(n-i)
    int binom = binomialCoefficient(n, i);
    double ti = std::pow(t, i);
    double ti1 = std::pow(1.0 - t, n - i);

    return binom * ti * ti1;
}

} // namespace cagd
