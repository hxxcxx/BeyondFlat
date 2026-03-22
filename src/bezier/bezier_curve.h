#pragma once

// Bezier Curve implementation for CAGD
// This module provides 2D and 3D Bezier curve classes

#include "cagd_types.h"

namespace cagd {

// ============================================================================
// Bezier Curve (2D)
// ============================================================================

class BezierCurve2d {
public:
    // Constructor with control points
    explicit BezierCurve2d(const PointVector2d& controlPoints);

    // Evaluate curve at parameter t in [0, 1]
    Point2d evaluate(double t) const;

    // Evaluate using De Casteljau algorithm (more numerically stable)
    Point2d deCasteljau(double t) const;

    // Compute derivative at parameter t
    Point2d derivative(double t, int order = 1) const;

    // Get degree of the curve
    int degree() const { return static_cast<int>(control_points_.size()) - 1; }

    // Get control points
    const PointVector2d& controlPoints() const { return control_points_; }

    // Set control points
    void setControlPoints(const PointVector2d& points) { control_points_ = points; }

private:
    PointVector2d control_points_;
};

// ============================================================================
// Bezier Curve (3D)
// ============================================================================

class BezierCurve3d {
public:
    // Constructor with control points
    explicit BezierCurve3d(const PointVector3d& controlPoints);

    // Evaluate curve at parameter t in [0, 1]
    Point3d evaluate(double t) const;

    // Evaluate using De Casteljau algorithm
    Point3d deCasteljau(double t) const;

    // Compute derivative at parameter t
    Point3d derivative(double t, int order = 1) const;

    // Get degree of the curve
    int degree() const { return static_cast<int>(control_points_.size()) - 1; }

    // Get control points
    const PointVector3d& controlPoints() const { return control_points_; }

    // Set control points
    void setControlPoints(const PointVector3d& points) { control_points_ = points; }

private:
    PointVector3d control_points_;
};

// ============================================================================
// Utility Functions for Bezier Curves
// ============================================================================

// Compute binomial coefficient C(n, k)
int binomialCoefficient(int n, int k);

// Compute Bernstein polynomial B_{i,n}(t)
double bernsteinPolynomial(int i, int n, double t);

} // namespace cagd
