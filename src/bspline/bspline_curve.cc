#define NOMINMAX
#include "bspline_curve.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <windows.h>

namespace cagd {

// ============================================================================
// BSplineCurve2d Implementation
// ============================================================================

BSplineCurve2d::BSplineCurve2d(int degree, const PointVector2d& controlPoints)
    : degree_(degree), control_points_(controlPoints) {
    knots_ = clampedKnotVector(degree, static_cast<int>(controlPoints.size()));
}

BSplineCurve2d::BSplineCurve2d(int degree, const PointVector2d& controlPoints,
                                const std::vector<double>& knots)
    : degree_(degree), control_points_(controlPoints), knots_(knots) {
}

void BSplineCurve2d::setControlPoints(const PointVector2d& points) {
    int oldCount = static_cast<int>(control_points_.size());
    control_points_ = points;
    int newCount = static_cast<int>(points.size());
    // Recompute knot vector if control point count changed
    // For clamped B-spline: knotCount = controlPointCount + degree + 1
    if (newCount + degree_ + 1 != knotCount()) {
        knots_ = clampedKnotVector(degree_, newCount);
    }
}

void BSplineCurve2d::setKnots(const std::vector<double>& knots) {
    knots_ = knots;
}

double BSplineCurve2d::basisFunction(int i, int p, double t) const {
    // Cox-de Boor recursion
    // N_{i,0}(t) = 1 if t_i <= t < t_{i+1}, else 0
    // N_{i,p}(t) = (t - t_i)/(t_{i+p} - t_i) * N_{i,p-1}(t) 
    //            + (t_{i+p+1} - t)/(t_{i+p+1} - t_{i+1}) * N_{i+1,p-1}(t)

    if (p == 0) {
        if (t >= knots_[i] && t < knots_[i + 1]) {
            return 1.0;
        }
        return 0.0;
    }

    double result = 0.0;

    // First term
    if (std::abs(knots_[i + p] - knots_[i]) > 1e-12) {
        result += (t - knots_[i]) / (knots_[i + p] - knots_[i]) * basisFunction(i, p - 1, t);
    }

    // Second term
    if (std::abs(knots_[i + p + 1] - knots_[i + 1]) > 1e-12) {
        result += (knots_[i + p + 1] - t) / (knots_[i + p + 1] - knots_[i + 1]) * basisFunction(i + 1, p - 1, t);
    }

    return result;
}

Point2d BSplineCurve2d::deBoor(double t) const {
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    // Find knot span
    int k = p;
    for (int i = p; i < n; ++i) {
        if (t >= knots_[i] && t < knots_[i + 1]) {
            k = i;
            break;
        }
    }
    // Handle endpoint case
    if (t >= knots_[n + 1]) {
        k = n;
    }

    // Copy control points for the de Boor algorithm
    std::vector<Point2d> d(p + 1);
    for (int j = 0; j <= p; ++j) {
        d[j] = control_points_[k - p + j];
    }

    // De Boor recursion
    for (int r = 1; r <= p; ++r) {
        for (int j = p; j >= r; --j) {
            int idx = k - p + j;
            double alpha = 0.0;
            if (std::abs(knots_[idx + p + 1 - r] - knots_[idx]) > 1e-12) {
                alpha = (t - knots_[idx]) / (knots_[idx + p + 1 - r] - knots_[idx]);
            }
            d[j] = (1.0 - alpha) * d[j - 1] + alpha * d[j];
        }
    }

    return d[p];
}

Point2d BSplineCurve2d::evaluate(double t) const {
    if (control_points_.empty()) {
        return Point2d(0.0, 0.0);
    }

    // Clamp t to domain
    auto [tmin, tmax] = domain();

    // Debug logging for domain issues
    if (t < tmin || t > tmax) {
        char buf[256];
        snprintf(buf, sizeof(buf), "evaluate: t=%.3f clamped from [%.3f, %.3f]\n", t, tmin, tmax);
        OutputDebugStringA(buf);
    }

    t = std::max(tmin, std::min(tmax, t));

    // Handle clamped endpoint
    if (t >= tmax) {
        return control_points_.back();
    }

    Point2d result = deBoor(t);
    return result;
}

Point2d BSplineCurve2d::derivative(double t, int order) const {
    if (order < 1) {
        return Point2d(0.0, 0.0);
    }

    // Clamp t to domain
    auto [tmin, tmax] = domain();
    t = std::max(tmin, std::min(tmax, t));

    const int p = degree_;
    if (order > p) {
        return Point2d(0.0, 0.0);  // Higher order derivatives beyond degree are zero
    }

    // Approximate derivative using central difference
    double h = 1e-5;
    Point2d d1 = (evaluate(t + h) - evaluate(t - h)) / (2.0 * h);
    Point2d d2 = (evaluate(t + h) - 2.0 * evaluate(t) + evaluate(t - h)) / (h * h);

    if (order == 1) return d1;
    if (order == 2) return d2;

    return Point2d(0.0, 0.0);
}

PointVector2d BSplineCurve2d::insertKnot(double t, int multiplicity) {
    // Knot insertion algorithm (Boehm's algorithm)
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    // Find knot span
    int k = p;
    for (int i = p; i < n; ++i) {
        if (t >= knots_[i] && t < knots_[i + 1]) {
            k = i;
            break;
        }
    }

    // New control points
    PointVector2d newPoints;
    newPoints.reserve(n + 2);

    // Copy first (k - p + 1) control points
    for (int i = 0; i <= k - p; ++i) {
        newPoints.push_back(control_points_[i]);
    }

    // Compute new control points
    for (int i = k - p + 1; i <= k - multiplicity; ++i) {
        double alpha = 0.0;
        if (std::abs(knots_[i + p] - knots_[i]) > 1e-12) {
            alpha = (t - knots_[i]) / (knots_[i + p] - knots_[i]);
        }
        newPoints.push_back((1.0 - alpha) * control_points_[i - 1] + alpha * control_points_[i]);
    }

    // Copy remaining control points
    for (int i = k - multiplicity + 1; i <= n; ++i) {
        newPoints.push_back(control_points_[i]);
    }

    // Insert knot multiplicity times
    std::vector<double> newKnots = knots_;
    for (int m = 0; m < multiplicity; ++m) {
        auto it = newKnots.begin() + k + 1 + m;
        newKnots.insert(it, t);
    }

    // Update curve
    control_points_ = newPoints;
    knots_ = newKnots;

    return newPoints;
}

std::pair<double, double> BSplineCurve2d::domain() const {
    if (knots_.size() < 2) {
        return {0.0, 1.0};
    }
    return {knots_[degree_], knots_[knots_.size() - degree_ - 1]};
}

bool BSplineCurve2d::isValid() const {
    int expectedKnots = control_points_.size() + degree_ + 1;
    return degree_ >= 0 && 
           !control_points_.empty() && 
           static_cast<int>(knots_.size()) == expectedKnots;
}

// ============================================================================
// BSplineCurve3d Implementation
// ============================================================================

BSplineCurve3d::BSplineCurve3d(int degree, const PointVector3d& controlPoints)
    : degree_(degree), control_points_(controlPoints) {
    knots_ = clampedKnotVector(degree, static_cast<int>(controlPoints.size()));
}

BSplineCurve3d::BSplineCurve3d(int degree, const PointVector3d& controlPoints,
                                const std::vector<double>& knots)
    : degree_(degree), control_points_(controlPoints), knots_(knots) {
}

void BSplineCurve3d::setControlPoints(const PointVector3d& points) {
    control_points_ = points;
    // Recompute knot vector if control point count changed
    // For clamped B-spline: knotCount = controlPointCount + degree + 1
    if (static_cast<int>(points.size()) + degree_ + 1 != knotCount()) {
        knots_ = clampedKnotVector(degree_, static_cast<int>(points.size()));
    }
}

void BSplineCurve3d::setKnots(const std::vector<double>& knots) {
    knots_ = knots;
}

double BSplineCurve3d::basisFunction(int i, int p, double t) const {
    if (p == 0) {
        if (t >= knots_[i] && t < knots_[i + 1]) {
            return 1.0;
        }
        return 0.0;
    }

    double result = 0.0;

    if (std::abs(knots_[i + p] - knots_[i]) > 1e-12) {
        result += (t - knots_[i]) / (knots_[i + p] - knots_[i]) * basisFunction(i, p - 1, t);
    }

    if (std::abs(knots_[i + p + 1] - knots_[i + 1]) > 1e-12) {
        result += (knots_[i + p + 1] - t) / (knots_[i + p + 1] - knots_[i + 1]) * basisFunction(i + 1, p - 1, t);
    }

    return result;
}

Point3d BSplineCurve3d::deBoor(double t) const {
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    int k = p;
    for (int i = p; i < n; ++i) {
        if (t >= knots_[i] && t < knots_[i + 1]) {
            k = i;
            break;
        }
    }
    if (t >= knots_[n + 1]) {
        k = n;
    }

    std::vector<Point3d> d(p + 1);
    for (int j = 0; j <= p; ++j) {
        d[j] = control_points_[k - p + j];
    }

    for (int r = 1; r <= p; ++r) {
        for (int j = p; j >= r; --j) {
            int idx = k - p + j;
            double alpha = 0.0;
            if (std::abs(knots_[idx + p + 1 - r] - knots_[idx]) > 1e-12) {
                alpha = (t - knots_[idx]) / (knots_[idx + p + 1 - r] - knots_[idx]);
            }
            d[j] = (1.0 - alpha) * d[j - 1] + alpha * d[j];
        }
    }

    return d[p];
}

Point3d BSplineCurve3d::evaluate(double t) const {
    if (control_points_.empty()) {
        return Point3d(0.0, 0.0, 0.0);
    }

    auto [tmin, tmax] = domain();
    t = std::max(tmin, std::min(tmax, t));

    if (t >= tmax) {
        return control_points_.back();
    }

    return deBoor(t);
}

Point3d BSplineCurve3d::derivative(double t, int order) const {
    if (order < 1) {
        return Point3d(0.0, 0.0, 0.0);
    }

    auto [tmin, tmax] = domain();
    t = std::max(tmin, std::min(tmax, t));

    const int p = degree_;
    if (order > p) {
        return Point3d(0.0, 0.0, 0.0);
    }

    double h = 1e-5;
    Point3d d1 = (evaluate(t + h) - evaluate(t - h)) / (2.0 * h);
    Point3d d2 = (evaluate(t + h) - 2.0 * evaluate(t) + evaluate(t - h)) / (h * h);

    if (order == 1) return d1;
    if (order == 2) return d2;

    return Point3d(0.0, 0.0, 0.0);
}

PointVector3d BSplineCurve3d::insertKnot(double t, int multiplicity) {
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    int k = p;
    for (int i = p; i < n; ++i) {
        if (t >= knots_[i] && t < knots_[i + 1]) {
            k = i;
            break;
        }
    }

    PointVector3d newPoints;
    newPoints.reserve(n + 2);

    for (int i = 0; i <= k - p; ++i) {
        newPoints.push_back(control_points_[i]);
    }

    for (int i = k - p + 1; i <= k - multiplicity; ++i) {
        double alpha = 0.0;
        if (std::abs(knots_[i + p] - knots_[i]) > 1e-12) {
            alpha = (t - knots_[i]) / (knots_[i + p] - knots_[i]);
        }
        newPoints.push_back((1.0 - alpha) * control_points_[i - 1] + alpha * control_points_[i]);
    }

    for (int i = k - multiplicity + 1; i <= n; ++i) {
        newPoints.push_back(control_points_[i]);
    }

    std::vector<double> newKnots = knots_;
    for (int m = 0; m < multiplicity; ++m) {
        auto it = newKnots.begin() + k + 1 + m;
        newKnots.insert(it, t);
    }

    control_points_ = newPoints;
    knots_ = newKnots;

    return newPoints;
}

std::pair<double, double> BSplineCurve3d::domain() const {
    if (knots_.size() < 2) {
        return {0.0, 1.0};
    }
    return {knots_[degree_], knots_[knots_.size() - degree_ - 1]};
}

bool BSplineCurve3d::isValid() const {
    int expectedKnots = control_points_.size() + degree_ + 1;
    return degree_ >= 0 && 
           !control_points_.empty() && 
           static_cast<int>(knots_.size()) == expectedKnots;
}

// ============================================================================
// Utility Functions
// ============================================================================

std::vector<double> uniformKnotVector(int degree, int controlPointCount) {
    int n = controlPointCount - 1;  // n + 1 control points
    int m = n + degree + 1;         // Number of knots

    std::vector<double> knots(m + 1, 0.0);

    // For uniform knots: spacing is constant
    double spacing = 1.0 / (n + degree + 1);
    for (int i = 0; i <= m; ++i) {
        knots[i] = i * spacing;
    }

    return knots;
}

std::vector<double> clampedKnotVector(int degree, int controlPointCount) {
    // Clamped (open) knot vector for B-spline with n+1 control points and degree p
    // Total knots needed: n + p + 2 = (controlPointCount - 1) + degree + 2
    // = controlPointCount + degree + 1
    //
    // Structure: [0 x (p+1)] [interior x (n-p)] [1 x (p+1)]
    //            = (p+1) + (n-p) + (p+1) = n + p + 2

    int n = controlPointCount - 1;  // n + 1 control points
    int numInteriorKnots = n - degree;  // Number of interior knots

    std::vector<double> knots;
    knots.reserve(controlPointCount + degree + 1);  // n + p + 2

    // First (degree + 1) knots are 0
    for (int i = 0; i <= degree; ++i) {
        knots.push_back(0.0);
    }

    // Interior knots are uniformly spaced between 0 and 1
    for (int i = 1; i <= numInteriorKnots; ++i) {
        double t = static_cast<double>(i) / (numInteriorKnots + 1);
        knots.push_back(t);
    }

    // Last (degree + 1) knots are 1
    for (int i = 0; i <= degree; ++i) {
        knots.push_back(1.0);
    }

    return knots;
}

} // namespace cagd
