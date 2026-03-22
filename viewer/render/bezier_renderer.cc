#include "bezier_renderer.h"
#include <windows.h>
#include <GL/gl.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cagd {

BezierRenderer::BezierRenderer()
    : CurveRenderer()
    , curve_(nullptr)
    , showTangent_(false)
    , showPointOnCurve_(false)
    , tangentParam_(0.5)
    , pointOnCurveParam_(0.5) {
    // Set default screen dimensions
    screenWidth_ = 1280;
    screenHeight_ = 720;

    // Set default world bounds
    worldMinX_ = -100;
    worldMaxX_ = 100;
    worldMinY_ = -100;
    worldMaxY_ = 100;
}

void BezierRenderer::renderCurve(const BezierCurve2d& curve, int segments,
                                 float r, float g, float b, float a) {
    glBegin(GL_LINE_STRIP);
    glColor4f(r, g, b, a);

    for (int i = 0; i <= segments; ++i) {
        double t = static_cast<double>(i) / segments;
        Point2d point = curve.evaluate(t);
        float sx, sy;
        worldToScreen(point, sx, sy);
        glVertex2f(sx, sy);
    }

    glEnd();
}

void BezierRenderer::renderControlPoints(const PointVector2d& points, float size,
                                        float r, float g, float b, float a) {
    for (const auto& point : points) {
        float sx, sy;
        worldToScreen(point, sx, sy);

        // Draw filled circle
        glColor4f(r, g, b, a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sx, sy);
        for (int i = 0; i <= 20; ++i) {
            float angle = 2.0f * M_PI * i / 20.0f;
            float px = sx + size * std::cos(angle);
            float py = sy + size * std::sin(angle);
            glVertex2f(px, py);
        }
        glEnd();

        // Draw border
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i <= 20; ++i) {
            float angle = 2.0f * M_PI * i / 20.0f;
            float px = sx + size * std::cos(angle);
            float py = sy + size * std::sin(angle);
            glVertex2f(px, py);
        }
        glEnd();
    }
}

void BezierRenderer::renderControlPolygon(const PointVector2d& points,
                                         float r, float g, float b, float a) {
    if (points.size() < 2) return;

    glLineStipple(2, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);

    glBegin(GL_LINE_STRIP);
    glColor4f(r, g, b, a);

    for (const auto& point : points) {
        float sx, sy;
        worldToScreen(point, sx, sy);
        glVertex2f(sx, sy);
    }

    glEnd();

    glDisable(GL_LINE_STIPPLE);
}

void BezierRenderer::renderTangent(const BezierCurve2d& curve, double t, float length,
                                  float r, float g, float b, float a) {
    Point2d point = curve.evaluate(t);
    Point2d tangent = curve.derivative(t);

    // Normalize tangent
    double norm = tangent.norm();
    if (norm > 1e-10) {
        tangent = (tangent / norm) * length;
    }

    float sx, sy;
    worldToScreen(point, sx, sy);

    float ex, ey;
    worldToScreen(point + tangent, ex, ey);

    // Draw tangent line
    glBegin(GL_LINES);
    glColor4f(r, g, b, a);
    glVertex2f(sx, sy);
    glVertex2f(ex, ey);
    glEnd();

    // Draw arrow head
    float angle = std::atan2(ey - sy, ex - sx);
    float arrowSize = 10.0f;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(ex, ey);
    glVertex2f(ex + arrowSize * std::cos(angle + 2.5), ey + arrowSize * std::sin(angle + 2.5));
    glVertex2f(ex + arrowSize * std::cos(angle - 2.5), ey + arrowSize * std::sin(angle - 2.5));
    glEnd();
}

void BezierRenderer::renderPointOnCurve(const BezierCurve2d& curve, double t, float size,
                                       float r, float g, float b, float a) {
    Point2d point = curve.evaluate(t);
    float sx, sy;
    worldToScreen(point, sx, sy);

    // Draw filled circle
    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(sx, sy);
    for (int i = 0; i <= 20; ++i) {
        float angle = 2.0f * M_PI * i / 20.0f;
        float px = sx + size * std::cos(angle);
        float py = sy + size * std::sin(angle);
        glVertex2f(px, py);
    }
    glEnd();

    // Draw border
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 20; ++i) {
        float angle = 2.0f * M_PI * i / 20.0f;
        float px = sx + size * std::cos(angle);
        float py = sy + size * std::sin(angle);
        glVertex2f(px, py);
    }
    glEnd();
}

std::vector<ColoredPoint> BezierRenderer::getCurvePoints(const BezierCurve2d& curve, int segments,
                                                         float r, float g, float b, float a) {
    std::vector<ColoredPoint> points;

    for (int i = 0; i <= segments; ++i) {
        double t = static_cast<double>(i) / segments;
        Point2d point = curve.evaluate(t);
        float sx, sy;
        worldToScreen(point, sx, sy);
        points.emplace_back(sx, sy, r, g, b, a);
    }

    return points;
}

std::vector<ColoredPoint> BezierRenderer::getControlPoints(const PointVector2d& points, float size,
                                                           float r, float g, float b, float a) {
    std::vector<ColoredPoint> result;

    for (const auto& point : points) {
        float sx, sy;
        worldToScreen(point, sx, sy);
        result.emplace_back(sx, sy, r, g, b, a);
    }

    return result;
}

std::vector<ColoredPoint> BezierRenderer::getControlPolygon(const PointVector2d& points,
                                                            float r, float g, float b, float a) {
    std::vector<ColoredPoint> result;

    for (const auto& point : points) {
        float sx, sy;
        worldToScreen(point, sx, sy);
        result.emplace_back(sx, sy, r, g, b, a);
    }

    return result;
}

// ============================================================================
// Base class interface implementations
// ============================================================================

void BezierRenderer::renderCurve() {
    if (curve_) {
        renderCurve(*curve_, 100);
    }
}

void BezierRenderer::renderControlPoints() {
    if (curve_) {
        renderControlPoints(curve_->controlPoints(), 10.0f);
    }
}

void BezierRenderer::renderControlPolygon() {
    if (curve_) {
        renderControlPolygon(curve_->controlPoints());
    }
}

void BezierRenderer::renderVisualizations() {
    if (!curve_) return;

    if (showTangent_) {
        renderTangent(*curve_, tangentParam_, 30.0f);
    }

    if (showPointOnCurve_) {
        renderPointOnCurve(*curve_, pointOnCurveParam_, 8.0f, 1.0f, 0.0f, 0.0f);
    }
}

} // namespace cagd
