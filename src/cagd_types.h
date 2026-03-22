#pragma once

// BeyondFlat - CAGD Learning Project
// Core type definitions and common utilities

#include <Eigen/Dense>
#include <vector>

namespace cagd {

// ============================================================================
// Type Aliases
// ============================================================================

// Vector types
using Vector2d = Eigen::Vector2d;
using Vector3d = Eigen::Vector3d;
using Vector4d = Eigen::Vector4d;

// Matrix types
using Matrix2d = Eigen::Matrix2d;
using Matrix3d = Eigen::Matrix3d;
using Matrix4d = Eigen::Matrix4d;
using MatrixXd = Eigen::MatrixXd;

// Dynamic size types
using VectorXd = Eigen::VectorXd;
using RowVectorXd = Eigen::RowVectorXd;

// Point types (for geometric operations)
using Point2d = Eigen::Vector2d;
using Point3d = Eigen::Vector3d;

// Container types for control points
using PointVector2d = std::vector<Point2d>;
using PointVector3d = std::vector<Point3d>;

} // namespace cagd
