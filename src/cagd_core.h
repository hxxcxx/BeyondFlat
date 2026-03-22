#pragma once

// BeyondFlat - CAGD Learning Project
// This library provides geometric operations for curves and surfaces

#include <Eigen/Dense>

namespace cagd {

// Convenient type aliases for CAGD operations
using Vector2d = Eigen::Vector2d;
using Vector3d = Eigen::Vector3d;
using Vector4d = Eigen::Vector4d;
using Matrix2d = Eigen::Matrix2d;
using Matrix3d = Eigen::Matrix3d;
using Matrix4d = Eigen::Matrix4d;
using MatrixXd = Eigen::MatrixXd;

// Dynamic size types for flexibility
using VectorXd = Eigen::VectorXd;
using RowVectorXd = Eigen::RowVectorXd;

// Point types (for geometric operations)
using Point2d = Eigen::Vector2d;
using Point3d = Eigen::Vector3d;

// Basic vector operations will be implemented here
// TODO: Implement vector classes and operations

// Basic matrix operations will be implemented here
// TODO: Implement matrix classes and operations

// Linear solvers will be implemented here
// TODO: Implement linear solvers

} // namespace cagd
