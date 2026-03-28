#pragma once

// Bezier Surface implementation for CAGD
// Tensor-product Bezier surface: S(u,v) = sum_i sum_j B_i^n(u) * B_j^m(v) * P_ij

#include "cagd_types.h"

namespace cagd {

class BezierSurface {
public:
    // Constructor with control point grid [row][col]
    // rows = (m+1) curves in v direction, cols = (n+1) control points per curve
    explicit BezierSurface(const SurfacePointGrid3d& controlPoints);

    // Evaluate surface at (u, v)
    Point3d evaluate(double u, double v) const;

    // Evaluate using de Casteljau (more numerically stable)
    Point3d deCasteljau(double u, double v) const;

    // Compute partial derivatives
    // Returns (dS/du, dS/dv) at (u, v)
    std::pair<Vector3d, Vector3d> derivatives(double u, double v) const;

    // Compute normal vector at (u, v)
    Vector3d normal(double u, double v) const;

    // Get degree in u and v directions
    int degreeU() const { return static_cast<int>(control_points_[0].size()) - 1; }
    int degreeV() const { return static_cast<int>(control_points_.size()) - 1; }

    // Get control points
    const SurfacePointGrid3d& controlPoints() const { return control_points_; }

    // Set control points
    void setControlPoints(const SurfacePointGrid3d& points) { control_points_ = points; }

    // Get single control point
    Point3d controlPoint(int row, int col) const;

    // Set single control point
    void setControlPoint(int row, int col, const Point3d& point);

    // Number of control points in each direction
    int numRows() const { return static_cast<int>(control_points_.size()); }
    int numCols() const { return static_cast<int>(control_points_[0].size()); }

    // Generate mesh for rendering
    // resolutionU x resolutionV grid of quads, each split into 2 triangles
    Mesh generateMesh(int resolutionU = 32, int resolutionV = 32) const;

    // Generate control polygon wireframe mesh (lines)
    Mesh generateControlNet() const;

    // Degree elevation in u direction
    BezierSurface elevateDegreeU() const;

    // Degree elevation in v direction
    BezierSurface elevateDegreeV() const;

private:
    SurfacePointGrid3d control_points_;

    // Validate control point grid
    void validate() const;

    // Evaluate a row of control points at parameter u using de Casteljau
    static Point3d evaluateRow(const PointVector3d& row, double u);

    // Interpolate all rows at parameter v to get a curve in u, then evaluate at u
    Point3d deCasteljauCore(double u, double v) const;
};

} // namespace cagd
