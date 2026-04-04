#pragma once

// NURBS (Non-Uniform Rational B-Spline) Curve implementation for CAGD
// NURBS generalizes B-spline curves with weighted control points,
// enabling exact representation of conic sections (circles, ellipses, etc.)

#include "cagd_types.h"

namespace cagd {

// ============================================================================
// NURBS Curve (2D)
// ============================================================================

class NURBSCurve2d {
public:
    // Default constructor creates an empty curve
    NURBSCurve2d() : degree_(0) {}

    // Constructor with control points and uniform weights (= 1.0)
    NURBSCurve2d(int degree, const PointVector2d& controlPoints);

    // Constructor with explicit weights and clamped knot vector
    NURBSCurve2d(int degree, const PointVector2d& controlPoints,
                 const std::vector<double>& weights);

    // Full constructor with explicit knot vector
    NURBSCurve2d(int degree, const PointVector2d& controlPoints,
                 const std::vector<double>& weights,
                 const std::vector<double>& knots);

    // Evaluate curve at parameter t using de Boor algorithm on homogeneous coords
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

    // Get weights
    const std::vector<double>& weights() const { return weights_; }

    // Set weights
    void setWeights(const std::vector<double>& weights);

    // Get knot vector
    const std::vector<double>& knots() const { return knots_; }

    // Set knot vector
    void setKnots(const std::vector<double>& knots);

    // Insert a knot at parameter t with given multiplicity
    PointVector2d insertKnot(double t, int multiplicity = 1);

    // Get knot vector domain (valid parameter range)
    std::pair<double, double> domain() const;

    // Check if curve is valid
    bool isValid() const;

    // Refine knot vector by inserting midpoints of each knot span
    // Returns the refined curve
    void refineKnotVector();

private:
    // Evaluate using de Boor algorithm in homogeneous coordinates
    // Returns (x*w, y*w, w) - the homogeneous point
    Vector3d deBoorHomogeneous(double t) const;

    int degree_;
    PointVector2d control_points_;
    std::vector<double> weights_;
    std::vector<double> knots_;
};

// ============================================================================
// NURBS Curve (3D)
// ============================================================================

class NURBSCurve3d {
public:
    // Default constructor creates an empty curve
    NURBSCurve3d() : degree_(0) {}

    // Constructor with control points and uniform weights (= 1.0)
    NURBSCurve3d(int degree, const PointVector3d& controlPoints);

    // Constructor with explicit weights and clamped knot vector
    NURBSCurve3d(int degree, const PointVector3d& controlPoints,
                 const std::vector<double>& weights);

    // Full constructor with explicit knot vector
    NURBSCurve3d(int degree, const PointVector3d& controlPoints,
                 const std::vector<double>& weights,
                 const std::vector<double>& knots);

    // Evaluate curve at parameter t
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

    // Get weights
    const std::vector<double>& weights() const { return weights_; }

    // Set weights
    void setWeights(const std::vector<double>& weights);

    // Get knot vector
    const std::vector<double>& knots() const { return knots_; }

    // Set knot vector
    void setKnots(const std::vector<double>& knots);

    // Insert a knot at parameter t
    PointVector3d insertKnot(double t, int multiplicity = 1);

    // Get knot vector domain
    std::pair<double, double> domain() const;

    // Check if curve is valid
    bool isValid() const;

    // Refine knot vector
    void refineKnotVector();

private:
    // Evaluate in homogeneous coordinates (x*w, y*w, z*w, w)
    Vector4d deBoorHomogeneous(double t) const;

    int degree_;
    PointVector3d control_points_;
    std::vector<double> weights_;
    std::vector<double> knots_;
};

} // namespace cagd
