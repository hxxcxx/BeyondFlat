#pragma once

// Common utility functions for CAGD operations

#include "cagd_types.h"

namespace cagd {

// ============================================================================
// Basic Geometric Operations
// ============================================================================

// Compute distance between two 3D points
double distance(const Point3d& p1, const Point3d& p2);

// Compute distance between two 2D points
double distance(const Point2d& p1, const Point2d& p2);

// Compute the midpoint of two 3D points
Point3d midpoint(const Point3d& p1, const Point3d& p2);

// Compute the midpoint of two 2D points
Point2d midpoint(const Point2d& p1, const Point2d& p2);

// ============================================================================
// Matrix Transformations
// ============================================================================

// Create a rotation matrix around X-axis
Matrix3d rotationX(double angle);

// Create a rotation matrix around Y-axis
Matrix3d rotationY(double angle);

// Create a rotation matrix around Z-axis
Matrix3d rotationZ(double angle);

// Create a translation matrix (4x4 for homogeneous coordinates)
Matrix4d translation(const Vector3d& offset);

// Create a scale matrix
Matrix4d scale(double sx, double sy, double sz);

// ============================================================================
// Mathematical Utilities
// ============================================================================

// Convert degrees to radians
double degToRad(double degrees);

// Convert radians to degrees
double radToDeg(double radians);

// Clamp value to range [min, max]
double clamp(double value, double min, double max);

// Linear interpolation
double lerp(double a, double b, double t);

} // namespace cagd
