#pragma once

// B-Spline Curve implementation for CAGD
// This module provides 2D and 3D B-spline curve classes

#include "cagd_types.h"

namespace cagd {

// ============================================================================
// B-Spline Curve (2D)
// ============================================================================

class BSplineCurve2d {
public:
    // Default constructor creates an empty curve
    BSplineCurve2d() : degree_(0) {}

    // Constructor with control points and uniform knot vector
    // Number of knots = number of control points + degree + 1
    BSplineCurve2d(int degree, const PointVector2d& controlPoints);

    // Constructor with explicit knot vector
    // knot vector size must be: control_points.size() + degree + 1
    BSplineCurve2d(int degree, const PointVector2d& controlPoints, const std::vector<double>& knots);

    // Evaluate curve at parameter t using de Boor algorithm
    Point2d evaluate(double t) const;

    // Compute derivative of specified order at parameter t
    Point2d derivative(double t, int order = 1) const;

    // Get degree of the curve
    int degree() const { return degree_; }

    // Get number of control points
    int controlPointCount() const { return static_cast<int>(control_points_.size()); }

    // Get number of knots
    int knotCount() const { return static_cast<int>(knots_.size()); }

    // Get control points
    const PointVector2d& controlPoints() const { return control_points_; }

    // Set control points (recomputes knot vector if needed)
    void setControlPoints(const PointVector2d& points);

    // Get knot vector
    const std::vector<double>& knots() const { return knots_; }

    // Set knot vector (size must match: control_points.size() + degree + 1)
    void setKnots(const std::vector<double>& knots);

    // Insert a knot at parameter t with given multiplicity
    // Returns the new control point vector
    PointVector2d insertKnot(double t, int multiplicity = 1);

    // Get knot vector domain (valid parameter range)
    std::pair<double, double> domain() const;

    // Check if curve is valid (proper knot vector and control points)
    bool isValid() const;

private:
    // Compute B-spline basis function N_{i,p}(t) using Cox-de Boor recursion
    double basisFunction(int i, int p, double t) const;

    // Evaluate using de Boor algorithm
    Point2d deBoor(double t) const;

    int degree_;
    PointVector2d control_points_;
    std::vector<double> knots_;
};

// ============================================================================
// B-Spline Curve (3D)
// ============================================================================

class BSplineCurve3d {
public:
    // Default constructor creates an empty curve
    BSplineCurve3d() : degree_(0) {}

    // Constructor with control points and uniform knot vector
    BSplineCurve3d(int degree, const PointVector3d& controlPoints);

    // Constructor with explicit knot vector
    BSplineCurve3d(int degree, const PointVector3d& controlPoints, const std::vector<double>& knots);

    // Evaluate curve at parameter t using de Boor algorithm
    Point3d evaluate(double t) const;

    // Compute derivative of specified order at parameter t
    Point3d derivative(double t, int order = 1) const;

    // Get degree of the curve
    int degree() const { return degree_; }

    // Get number of control points
    int controlPointCount() const { return static_cast<int>(control_points_.size()); }

    // Get number of knots
    int knotCount() const { return static_cast<int>(knots_.size()); }

    // Get control points
    const PointVector3d& controlPoints() const { return control_points_; }

    // Set control points
    void setControlPoints(const PointVector3d& points);

    // Get knot vector
    const std::vector<double>& knots() const { return knots_; }

    // Set knot vector
    void setKnots(const std::vector<double>& knots);

    // Insert a knot at parameter t with given multiplicity
    PointVector3d insertKnot(double t, int multiplicity = 1);

    // Get knot vector domain
    std::pair<double, double> domain() const;

    // Check if curve is valid
    bool isValid() const;

private:
    double basisFunction(int i, int p, double t) const;
    Point3d deBoor(double t) const;

    int degree_;
    PointVector3d control_points_;
    std::vector<double> knots_;
};

// ============================================================================
// Utility Functions for B-Spline Curves
// ============================================================================

// Generate uniform (equally spaced) knot vector
// n + p + 2 knots for n+1 control points of degree p
std::vector<double> uniformKnotVector(int degree, int controlPointCount);

// Generate clamped (open) knot vector
// Ensures curve passes through first and last control points
std::vector<double> clampedKnotVector(int degree, int controlPointCount);

} // namespace cagd
