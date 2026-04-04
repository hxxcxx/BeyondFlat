#define NOMINMAX
#include "nurbs_curve.h"
#include "bspline/bspline_curve.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <windows.h>

namespace cagd {

// ============================================================================
// NURBSCurve2d Implementation
// ============================================================================

NURBSCurve2d::NURBSCurve2d(int degree, const PointVector2d& controlPoints)
    : degree_(degree), control_points_(controlPoints),
      weights_(controlPoints.size(), 1.0) {
    knots_ = clampedKnotVector(degree, static_cast<int>(controlPoints.size()));
}

NURBSCurve2d::NURBSCurve2d(int degree, const PointVector2d& controlPoints,
                           const std::vector<double>& weights)
    : degree_(degree), control_points_(controlPoints), weights_(weights) {
    knots_ = clampedKnotVector(degree, static_cast<int>(controlPoints.size()));
}

NURBSCurve2d::NURBSCurve2d(int degree, const PointVector2d& controlPoints,
                           const std::vector<double>& weights,
                           const std::vector<double>& knots)
    : degree_(degree), control_points_(controlPoints),
      weights_(weights), knots_(knots) {
}

void NURBSCurve2d::setControlPoints(const PointVector2d& points) {
    int oldCount = static_cast<int>(control_points_.size());
    control_points_ = points;
    int newCount = static_cast<int>(points.size());

    // Adjust weights size
    if (newCount != oldCount) {
        weights_.resize(newCount, 1.0);
        knots_ = clampedKnotVector(degree_, newCount);
    }
}

void NURBSCurve2d::setWeights(const std::vector<double>& weights) {
    if (static_cast<int>(weights.size()) == controlPointCount()) {
        weights_ = weights;
    }
}

void NURBSCurve2d::setKnots(const std::vector<double>& knots) {
    knots_ = knots;
}

Vector3d NURBSCurve2d::deBoorHomogeneous(double t) const {
    // NURBS evaluation in homogeneous coordinates:
    // Represent each control point as (w*x, w*y, w) and run de Boor
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    // Find knot span: k such that knots_[k] <= t < knots_[k+1]
    int k = n;
    for (int i = p; i <= n; ++i) {
        if (t < knots_[i + 1]) {
            k = i;
            break;
        }
    }

    // Copy homogeneous control points for the de Boor algorithm
    std::vector<Vector3d> d(p + 1);
    for (int j = 0; j <= p; ++j) {
        int idx = k - p + j;
        double w = weights_[idx];
        d[j] = Vector3d(control_points_[idx].x() * w,
                        control_points_[idx].y() * w,
                        w);
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

Point2d NURBSCurve2d::evaluate(double t) const {
    if (control_points_.empty()) {
        return Point2d(0.0, 0.0);
    }

    // Clamp t to domain
    auto [tmin, tmax] = domain();
    t = std::max(tmin, std::min(tmax, t));

    // Handle clamped endpoint
    if (t >= tmax) {
        return control_points_.back();
    }

    Vector3d h = deBoorHomogeneous(t);
    if (std::abs(h.z()) < 1e-15) {
        return Point2d(0.0, 0.0);
    }

    // Perspective divide: (w*x/w, w*y/w) = (x, y)
    return Point2d(h.x() / h.z(), h.y() / h.z());
}

Point2d NURBSCurve2d::derivative(double t, int order) const {
    if (order < 1) {
        return Point2d(0.0, 0.0);
    }

    auto [tmin, tmax] = domain();
    t = std::max(tmin, std::min(tmax, t));

    const int p = degree_;
    if (order > p) {
        return Point2d(0.0, 0.0);
    }

    // Numerical differentiation for NURBS
    double h = 1e-5;
    Point2d d1 = (evaluate(t + h) - evaluate(t - h)) / (2.0 * h);
    Point2d d2 = (evaluate(t + h) - 2.0 * evaluate(t) + evaluate(t - h)) / (h * h);

    if (order == 1) return d1;
    if (order == 2) return d2;

    return Point2d(0.0, 0.0);
}

PointVector2d NURBSCurve2d::insertKnot(double t, int multiplicity) {
    // Knot insertion for NURBS: work in homogeneous coordinates
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    // Find knot span
    int k = n;
    for (int i = p; i <= n; ++i) {
        if (t < knots_[i + 1]) {
            k = i;
            break;
        }
    }

    // New control points and weights
    PointVector2d newPoints;
    std::vector<double> newWeights;
    newPoints.reserve(n + 2);
    newWeights.reserve(n + 2);

    // Copy first (k - p + 1) control points
    for (int i = 0; i <= k - p; ++i) {
        newPoints.push_back(control_points_[i]);
        newWeights.push_back(weights_[i]);
    }

    // Compute new control points using Boehm's algorithm in homogeneous space
    for (int i = k - p + 1; i <= k - multiplicity; ++i) {
        double alpha = 0.0;
        if (std::abs(knots_[i + p] - knots_[i]) > 1e-12) {
            alpha = (t - knots_[i]) / (knots_[i + p] - knots_[i]);
        }

        // Interpolate in homogeneous space
        double w1 = weights_[i - 1], w2 = weights_[i];
        double newW = (1.0 - alpha) * w1 + alpha * w2;

        Point2d newP;
        if (std::abs(newW) > 1e-15) {
            newP = Point2d(
                ((1.0 - alpha) * w1 * control_points_[i - 1].x() + alpha * w2 * control_points_[i].x()) / newW,
                ((1.0 - alpha) * w1 * control_points_[i - 1].y() + alpha * w2 * control_points_[i].y()) / newW
            );
        } else {
            newP = (1.0 - alpha) * control_points_[i - 1] + alpha * control_points_[i];
        }

        newPoints.push_back(newP);
        newWeights.push_back(newW);
    }

    // Copy remaining control points
    for (int i = k - multiplicity + 1; i <= n; ++i) {
        newPoints.push_back(control_points_[i]);
        newWeights.push_back(weights_[i]);
    }

    // Insert knot
    std::vector<double> newKnots = knots_;
    for (int m = 0; m < multiplicity; ++m) {
        auto it = newKnots.begin() + k + 1 + m;
        newKnots.insert(it, t);
    }

    control_points_ = newPoints;
    weights_ = newWeights;
    knots_ = newKnots;

    return newPoints;
}

std::pair<double, double> NURBSCurve2d::domain() const {
    if (knots_.size() < 2) {
        return {0.0, 1.0};
    }
    return {knots_[degree_], knots_[knots_.size() - degree_ - 1]};
}

bool NURBSCurve2d::isValid() const {
    int expectedKnots = static_cast<int>(control_points_.size()) + degree_ + 1;
    return degree_ >= 0 &&
           !control_points_.empty() &&
           static_cast<int>(knots_.size()) == expectedKnots &&
           static_cast<int>(weights_.size()) == controlPointCount();
}

void NURBSCurve2d::refineKnotVector() {
    if (!isValid() || knots_.size() < 2) return;

    // Collect midpoints of each unique knot span
    std::vector<double> newKnots;
    for (size_t i = degree_; i < knots_.size() - degree_ - 1; ++i) {
        double mid = (knots_[i] + knots_[i + 1]) / 2.0;
        if (std::abs(knots_[i + 1] - knots_[i]) > 1e-12) {
            newKnots.push_back(mid);
        }
    }

    // Insert each new knot
    for (double t : newKnots) {
        insertKnot(t, 1);
    }
}

// ============================================================================
// NURBSCurve3d Implementation
// ============================================================================

NURBSCurve3d::NURBSCurve3d(int degree, const PointVector3d& controlPoints)
    : degree_(degree), control_points_(controlPoints),
      weights_(controlPoints.size(), 1.0) {
    knots_ = clampedKnotVector(degree, static_cast<int>(controlPoints.size()));
}

NURBSCurve3d::NURBSCurve3d(int degree, const PointVector3d& controlPoints,
                           const std::vector<double>& weights)
    : degree_(degree), control_points_(controlPoints), weights_(weights) {
    knots_ = clampedKnotVector(degree, static_cast<int>(controlPoints.size()));
}

NURBSCurve3d::NURBSCurve3d(int degree, const PointVector3d& controlPoints,
                           const std::vector<double>& weights,
                           const std::vector<double>& knots)
    : degree_(degree), control_points_(controlPoints),
      weights_(weights), knots_(knots) {
}

void NURBSCurve3d::setControlPoints(const PointVector3d& points) {
    int oldCount = static_cast<int>(control_points_.size());
    control_points_ = points;
    int newCount = static_cast<int>(points.size());

    if (newCount != oldCount) {
        weights_.resize(newCount, 1.0);
        knots_ = clampedKnotVector(degree_, newCount);
    }
}

void NURBSCurve3d::setWeights(const std::vector<double>& weights) {
    if (static_cast<int>(weights.size()) == controlPointCount()) {
        weights_ = weights;
    }
}

void NURBSCurve3d::setKnots(const std::vector<double>& knots) {
    knots_ = knots;
}

Vector4d NURBSCurve3d::deBoorHomogeneous(double t) const {
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    // Find knot span
    int k = n;
    for (int i = p; i <= n; ++i) {
        if (t < knots_[i + 1]) {
            k = i;
            break;
        }
    }

    // Copy homogeneous control points (w*x, w*y, w*z, w)
    std::vector<Vector4d> d(p + 1);
    for (int j = 0; j <= p; ++j) {
        int idx = k - p + j;
        double w = weights_[idx];
        d[j] = Vector4d(control_points_[idx].x() * w,
                        control_points_[idx].y() * w,
                        control_points_[idx].z() * w,
                        w);
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

Point3d NURBSCurve3d::evaluate(double t) const {
    if (control_points_.empty()) {
        return Point3d(0.0, 0.0, 0.0);
    }

    auto [tmin, tmax] = domain();
    t = std::max(tmin, std::min(tmax, t));

    if (t >= tmax) {
        return control_points_.back();
    }

    Vector4d h = deBoorHomogeneous(t);
    if (std::abs(h.w()) < 1e-15) {
        return Point3d(0.0, 0.0, 0.0);
    }

    return Point3d(h.x() / h.w(), h.y() / h.w(), h.z() / h.w());
}

Point3d NURBSCurve3d::derivative(double t, int order) const {
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

PointVector3d NURBSCurve3d::insertKnot(double t, int multiplicity) {
    const int n = static_cast<int>(control_points_.size()) - 1;
    const int p = degree_;

    int k = n;
    for (int i = p; i <= n; ++i) {
        if (t < knots_[i + 1]) {
            k = i;
            break;
        }
    }

    PointVector3d newPoints;
    std::vector<double> newWeights;
    newPoints.reserve(n + 2);
    newWeights.reserve(n + 2);

    for (int i = 0; i <= k - p; ++i) {
        newPoints.push_back(control_points_[i]);
        newWeights.push_back(weights_[i]);
    }

    for (int i = k - p + 1; i <= k - multiplicity; ++i) {
        double alpha = 0.0;
        if (std::abs(knots_[i + p] - knots_[i]) > 1e-12) {
            alpha = (t - knots_[i]) / (knots_[i + p] - knots_[i]);
        }

        double w1 = weights_[i - 1], w2 = weights_[i];
        double newW = (1.0 - alpha) * w1 + alpha * w2;

        Point3d newP;
        if (std::abs(newW) > 1e-15) {
            newP = Point3d(
                ((1.0 - alpha) * w1 * control_points_[i - 1].x() + alpha * w2 * control_points_[i].x()) / newW,
                ((1.0 - alpha) * w1 * control_points_[i - 1].y() + alpha * w2 * control_points_[i].y()) / newW,
                ((1.0 - alpha) * w1 * control_points_[i - 1].z() + alpha * w2 * control_points_[i].z()) / newW
            );
        } else {
            newP = (1.0 - alpha) * control_points_[i - 1] + alpha * control_points_[i];
        }

        newPoints.push_back(newP);
        newWeights.push_back(newW);
    }

    for (int i = k - multiplicity + 1; i <= n; ++i) {
        newPoints.push_back(control_points_[i]);
        newWeights.push_back(weights_[i]);
    }

    std::vector<double> newKnots = knots_;
    for (int m = 0; m < multiplicity; ++m) {
        auto it = newKnots.begin() + k + 1 + m;
        newKnots.insert(it, t);
    }

    control_points_ = newPoints;
    weights_ = newWeights;
    knots_ = newKnots;

    return newPoints;
}

std::pair<double, double> NURBSCurve3d::domain() const {
    if (knots_.size() < 2) {
        return {0.0, 1.0};
    }
    return {knots_[degree_], knots_[knots_.size() - degree_ - 1]};
}

bool NURBSCurve3d::isValid() const {
    int expectedKnots = static_cast<int>(control_points_.size()) + degree_ + 1;
    return degree_ >= 0 &&
           !control_points_.empty() &&
           static_cast<int>(knots_.size()) == expectedKnots &&
           static_cast<int>(weights_.size()) == controlPointCount();
}

void NURBSCurve3d::refineKnotVector() {
    if (!isValid() || knots_.size() < 2) return;

    std::vector<double> newKnots;
    for (size_t i = degree_; i < knots_.size() - degree_ - 1; ++i) {
        double mid = (knots_[i] + knots_[i + 1]) / 2.0;
        if (std::abs(knots_[i + 1] - knots_[i]) > 1e-12) {
            newKnots.push_back(mid);
        }
    }

    for (double t : newKnots) {
        insertKnot(t, 1);
    }
}

} // namespace cagd
