#pragma once

// BeyondFlat - CAGD Learning Project
// Core type definitions and common utilities

#include <Eigen/Dense>
#include <vector>
#include <optional>
#include <cstdint>

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

// Container type for surface control point grid: controlPoints_[row][col]
using SurfacePointGrid3d = std::vector<PointVector3d>;

// ============================================================================
// 3D Graphics Math Utilities
// ============================================================================

// Create a perspective projection matrix (OpenGL convention: maps to [-1,1] NDC)
Matrix4d perspectiveMatrix(double fovDeg, double aspect, double nearPlane, double farPlane);

// Create an orthographic projection matrix
Matrix4d orthoMatrix(double left, double right, double bottom, double top, double nearPlane, double farPlane);

// Create a look-at view matrix
Matrix4d lookAtMatrix(const Vector3d& eye, const Vector3d& center, const Vector3d& up);

// Create a translation matrix
Matrix4d translationMatrix(const Vector3d& offset);

// Create a rotation matrix around X-axis
Matrix4d rotationXMatrix(double angleRad);

// Create a rotation matrix around Y-axis
Matrix4d rotationYMatrix(double angleRad);

// Create a rotation matrix around Z-axis
Matrix4d rotationZMatrix(double angleRad);

// Create a uniform scale matrix
Matrix4d scaleMatrix(double s);

// ============================================================================
// Mesh Data Structures (for 3D rendering)
// ============================================================================

struct Vertex {
    Vector3d position;
    Vector3d normal;
    uint32_t color;  // ABGR packed (for ImGui/OpenGL compatibility)

    Vertex() : position(Vector3d::Zero()), normal(Vector3d::UnitZ()), color(0xFFFFFFFF) {}
    Vertex(const Vector3d& pos, const Vector3d& norm, uint32_t col = 0xFFFFFFFF)
        : position(pos), normal(norm), color(col) {}
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    bool empty() const { return vertices.empty(); }
    void clear() { vertices.clear(); indices.clear(); }
};

// Simple RGBA color
struct Color4f {
    float r, g, b, a;
    Color4f(float r = 1, float g = 1, float b = 1, float a = 1)
        : r(r), g(g), b(b), a(a) {}

    uint32_t toABGR() const {
        return (static_cast<uint32_t>(a * 255) << 24) |
               (static_cast<uint32_t>(b * 255) << 16) |
               (static_cast<uint32_t>(g * 255) << 8) |
               (static_cast<uint32_t>(r * 255));
    }
};

} // namespace cagd
