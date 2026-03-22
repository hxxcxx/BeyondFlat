#pragma once

// Bezier Curve Renderer for OpenGL visualization

#include "../../src/cagd_types.h"
#include "../../src/bezier/bezier_curve.h"
#include "../base/curve_renderer.h"
#include <vector>

namespace cagd {

// Structure to represent a 2D point with color
struct ColoredPoint {
    float x, y;
    float r, g, b, a;

    ColoredPoint(float px, float py, float pr = 1.0f, float pg = 1.0f, float pb = 1.0f, float pa = 1.0f)
        : x(px), y(py), r(pr), g(pg), b(pb), a(pa) {}
};

// Bezier Curve Renderer
class BezierRenderer : public CurveRenderer {
public:
    BezierRenderer();

    // Set the curve to render
    void setCurve(const BezierCurve2d& curve) { curve_ = &curve; }

    // Render the Bezier curve
    void renderCurve(const BezierCurve2d& curve, int segments = 100,
                     float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

    // Render control points
    void renderControlPoints(const PointVector2d& points, float size = 8.0f,
                            float r = 1.0f, float g = 0.5f, float b = 0.0f, float a = 1.0f);

    // Render control polygon (lines connecting control points)
    void renderControlPolygon(const PointVector2d& points,
                             float r = 0.5f, float g = 0.5f, float b = 0.5f, float a = 0.5f);

    // Render tangent at parameter t
    void renderTangent(const BezierCurve2d& curve, double t, float length = 50.0f,
                      float r = 0.0f, float g = 1.0f, float b = 0.0f, float a = 1.0f);

    // Render point on curve at parameter t
    void renderPointOnCurve(const BezierCurve2d& curve, double t, float size = 10.0f,
                           float r = 1.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

    // Implementation of base class interface
    void renderCurve() override;
    void renderControlPoints() override;
    void renderControlPolygon() override;
    void renderVisualizations() override;
    std::string getName() const override { return "Bezier"; }

    // Get curve points for rendering
    std::vector<ColoredPoint> getCurvePoints(const BezierCurve2d& curve, int segments = 100,
                                             float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

    // Get control points for rendering
    std::vector<ColoredPoint> getControlPoints(const PointVector2d& points, float size = 8.0f,
                                               float r = 1.0f, float g = 0.5f, float b = 0.0f, float a = 1.0f);

    // Get control polygon lines for rendering
    std::vector<ColoredPoint> getControlPolygon(const PointVector2d& points,
                                                float r = 0.5f, float g = 0.5f, float b = 0.5f, float a = 0.5f);

    // Visualization options
    void setShowTangent(bool show) { showTangent_ = show; }
    void setShowPointOnCurve(bool show) { showPointOnCurve_ = show; }
    void setTangentParam(double t) { tangentParam_ = t; }
    void setPointOnCurveParam(double t) { pointOnCurveParam_ = t; }

private:
    // Pointer to the current curve (not owned)
    const BezierCurve2d* curve_;

    // Visualization options
    bool showTangent_;
    bool showPointOnCurve_;
    double tangentParam_;
    double pointOnCurveParam_;
};

} // namespace cagd
