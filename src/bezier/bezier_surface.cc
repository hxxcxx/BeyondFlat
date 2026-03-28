#include "bezier_surface.h"
#include "bezier_curve.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace cagd {

BezierSurface::BezierSurface(const SurfacePointGrid3d& controlPoints)
    : control_points_(controlPoints) {
    validate();
}

void BezierSurface::validate() const {
    if (control_points_.empty()) {
        throw std::invalid_argument("Control point grid cannot be empty");
    }
    size_t cols = control_points_[0].size();
    if (cols == 0) {
        throw std::invalid_argument("Control point rows cannot be empty");
    }
    for (const auto& row : control_points_) {
        if (row.size() != cols) {
            throw std::invalid_argument("All rows must have the same number of control points");
        }
    }
}

Point3d BezierSurface::evaluate(double u, double v) const {
    u = std::max(0.0, std::min(1.0, u));
    v = std::max(0.0, std::min(1.0, v));

    int n = degreeU();
    int m = degreeV();

    Point3d point = Point3d::Zero();

    for (int j = 0; j <= m; ++j) {
        Point3d rowPoint = Point3d::Zero();
        double bj = bernsteinPolynomial(j, m, v);

        for (int i = 0; i <= n; ++i) {
            double bi = bernsteinPolynomial(i, n, u);
            rowPoint += bi * control_points_[j][i];
        }
        point += bj * rowPoint;
    }

    return point;
}

Point3d BezierSurface::deCasteljau(double u, double v) const {
    return deCasteljauCore(u, v);
}

Point3d BezierSurface::deCasteljauCore(double u, double v) const {
    u = std::max(0.0, std::min(1.0, u));
    v = std::max(0.0, std::min(1.0, v));

    // Step 1: For each row (iso-v curve), evaluate at u using de Casteljau
    int m = degreeV();
    PointVector3d curveAtU(numRows());
    for (int j = 0; j <= m; ++j) {
        curveAtU[j] = evaluateRow(control_points_[j], u);
    }

    // Step 2: Evaluate the resulting curve at v using de Casteljau
    return evaluateRow(curveAtU, v);
}

Point3d BezierSurface::evaluateRow(const PointVector3d& row, double t) {
    PointVector3d pts = row;
    int n = static_cast<int>(pts.size()) - 1;
    for (int k = 1; k <= n; ++k) {
        for (int i = 0; i <= n - k; ++i) {
            pts[i] = (1.0 - t) * pts[i] + t * pts[i + 1];
        }
    }
    return pts[0];
}

std::pair<Vector3d, Vector3d> BezierSurface::derivatives(double u, double v) const {
    u = std::max(0.0, std::min(1.0, u));
    v = std::max(0.0, std::min(1.0, v));

    int n = degreeU();
    int m = degreeV();

    // dS/du: differentiate control points in u direction, then evaluate
    PointVector3d duCurve(numRows());
    for (int j = 0; j <= m; ++j) {
        Point3d duRow = Point3d::Zero();
        for (int i = 0; i < n; ++i) {
            duRow += n * (control_points_[j][i + 1] - control_points_[j][i]) *
                     bernsteinPolynomial(i, n - 1, u);
        }
        duCurve[j] = duRow;
    }
    // Evaluate at v
    Vector3d dSdu = Point3d::Zero();
    for (int j = 0; j <= m; ++j) {
        dSdu += bernsteinPolynomial(j, m, v) * duCurve[j];
    }

    // dS/dv: differentiate control points in v direction, then evaluate
    PointVector3d dvCurve(n + 1);
    for (int i = 0; i <= n; ++i) {
        Point3d dvCol = Point3d::Zero();
        for (int j = 0; j < m; ++j) {
            dvCol += m * (control_points_[j + 1][i] - control_points_[j][i]) *
                     bernsteinPolynomial(j, m - 1, v);
        }
        dvCurve[i] = dvCol;
    }
    // Evaluate at u
    Vector3d dSdv = Point3d::Zero();
    for (int i = 0; i <= n; ++i) {
        dSdv += bernsteinPolynomial(i, n, u) * dvCurve[i];
    }

    return {dSdu, dSdv};
}

Vector3d BezierSurface::normal(double u, double v) const {
    auto [dSdu, dSdv] = derivatives(u, v);
    Vector3d n = dSdu.cross(dSdv);
    double len = n.norm();
    if (len < 1e-12) {
        return Vector3d::UnitZ();
    }
    return n / len;
}

Point3d BezierSurface::controlPoint(int row, int col) const {
    return control_points_[row][col];
}

void BezierSurface::setControlPoint(int row, int col, const Point3d& point) {
    control_points_[row][col] = point;
}

Mesh BezierSurface::generateMesh(int resolutionU, int resolutionV) const {
    Mesh mesh;

    // Generate vertices
    for (int j = 0; j <= resolutionV; ++j) {
        double v = static_cast<double>(j) / resolutionV;
        for (int i = 0; i <= resolutionU; ++i) {
            double u = static_cast<double>(i) / resolutionU;

            Point3d pos = deCasteljau(u, v);
            Vector3d norm = normal(u, v);

            mesh.vertices.push_back(Vertex(pos, norm, Color4f(0.4f, 0.7f, 1.0f, 1.0f).toABGR()));
        }
    }

    int cols = resolutionU + 1;

    // Generate triangle indices
    for (int j = 0; j < resolutionV; ++j) {
        for (int i = 0; i < resolutionU; ++i) {
            uint32_t p00 = j * cols + i;
            uint32_t p10 = j * cols + (i + 1);
            uint32_t p01 = (j + 1) * cols + i;
            uint32_t p11 = (j + 1) * cols + (i + 1);

            // Two triangles per quad
            mesh.indices.push_back(p00);
            mesh.indices.push_back(p10);
            mesh.indices.push_back(p11);

            mesh.indices.push_back(p00);
            mesh.indices.push_back(p11);
            mesh.indices.push_back(p01);
        }
    }

    return mesh;
}

Mesh BezierSurface::generateControlNet() const {
    Mesh mesh;
    int rows = numRows();
    int cols = numCols();
    uint32_t color = Color4f(1.0f, 0.65f, 0.0f, 1.0f).toABGR();
    Vector3d up(0, 0, 1);

    // Lines along u direction (each row)
    for (int j = 0; j < rows; ++j) {
        for (int i = 0; i < cols - 1; ++i) {
            uint32_t idx = static_cast<uint32_t>(mesh.vertices.size());
            mesh.vertices.push_back(Vertex(control_points_[j][i], up, color));
            mesh.vertices.push_back(Vertex(control_points_[j][i + 1], up, color));
            mesh.indices.push_back(idx);
            mesh.indices.push_back(idx + 1);
        }
    }

    // Lines along v direction (each column)
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < rows - 1; ++j) {
            uint32_t idx = static_cast<uint32_t>(mesh.vertices.size());
            mesh.vertices.push_back(Vertex(control_points_[j][i], up, color));
            mesh.vertices.push_back(Vertex(control_points_[j + 1][i], up, color));
            mesh.indices.push_back(idx);
            mesh.indices.push_back(idx + 1);
        }
    }

    return mesh;
}

BezierSurface BezierSurface::elevateDegreeU() const {
    int n = degreeU();
    int newN = n + 1;
    int m = degreeV();

    SurfacePointGrid3d newGrid;
    for (int j = 0; j <= m; ++j) {
        PointVector3d newRow(newN + 1);
        newRow[0] = control_points_[j][0];
        for (int i = 1; i <= n; ++i) {
            double alpha = static_cast<double>(i) / (newN);
            newRow[i] = alpha * control_points_[j][i - 1] + (1.0 - alpha) * control_points_[j][i];
        }
        newRow[newN] = control_points_[j][n];
        newGrid.push_back(newRow);
    }
    return BezierSurface(newGrid);
}

BezierSurface BezierSurface::elevateDegreeV() const {
    int m = degreeV();
    int newM = m + 1;
    int n = degreeU();

    SurfacePointGrid3d newGrid;
    for (int j = 0; j <= newM; ++j) {
        PointVector3d newRow(n + 1);
        if (j == 0) {
            newRow = control_points_[0];
        } else if (j == newM) {
            newRow = control_points_[m];
        } else {
            double alpha = static_cast<double>(j) / (newM);
            for (int i = 0; i <= n; ++i) {
                newRow[i] = alpha * control_points_[j - 1][i] + (1.0 - alpha) * control_points_[j][i];
            }
        }
        newGrid.push_back(newRow);
    }
    return BezierSurface(newGrid);
}

} // namespace cagd
