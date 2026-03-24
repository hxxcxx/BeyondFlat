#include "bezier_editor.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cagd {

BezierEditor::BezierEditor()
    : CurveEditor()  // Initialize base class first
    , showControlPoints_(true)
    , showControlPolygon_(true)
    , showTangent_(false)
    , showPointOnCurve_(false)
    , tangentParam_(0.5)
    , pointOnCurveParam_(0.5)
    , selectedControlPoint_(-1)
    , isDragging_(false)
    , controlPointSize_(10.0f) {

    // Set screen dimensions in base class
    screenWidth_ = 1280;
    screenHeight_ = 720;

    // Set world bounds in base class
    worldMinX_ = -100;
    worldMaxX_ = 100;
    worldMinY_ = -100;
    worldMaxY_ = 100;

    // Initialize with default control points (cubic Bezier)
    PointVector2d controlPoints;
    controlPoints.emplace_back(-50, -20);
    controlPoints.emplace_back(-30, 50);
    controlPoints.emplace_back(30, 50);
    controlPoints.emplace_back(50, -20);

    curve_ = std::make_unique<BezierCurve2d>(controlPoints);
    renderer_ = std::make_unique<BezierRenderer>();
}

void BezierEditor::initialize() {
    renderer_->setScreenSize(screenWidth_, screenHeight_);
    renderer_->setWorldBounds(worldMinX_, worldMaxX_, worldMinY_, worldMaxY_);

    // Set the curve in the renderer
    auto* bezierRenderer = static_cast<BezierRenderer*>(renderer_.get());
    bezierRenderer->setCurve(*curve_);
}

void BezierEditor::render() {
    // Update renderer options
    auto* bezierRenderer = static_cast<BezierRenderer*>(renderer_.get());
    bezierRenderer->setShowTangent(showTangent_);
    bezierRenderer->setShowPointOnCurve(showPointOnCurve_);
    bezierRenderer->setTangentParam(tangentParam_);
    bezierRenderer->setPointOnCurveParam(pointOnCurveParam_);

    // Render the curve and related elements
    if (showControlPolygon_) {
        renderer_->renderControlPolygon();
    }

    renderer_->renderCurve();

    if (showControlPoints_) {
        renderer_->renderControlPoints();
    }

    renderer_->renderVisualizations();
}

void BezierEditor::renderControlPanel() {
    ImGui::Begin("Bezier Curve Control Panel");

    // Display options
    ImGui::Checkbox("Show Control Points", &showControlPoints_);
    ImGui::Checkbox("Show Control Polygon", &showControlPolygon_);
    ImGui::Checkbox("Show Tangent", &showTangent_);
    ImGui::Checkbox("Show Point on Curve", &showPointOnCurve_);

    ImGui::Separator();

    // Tangent parameter slider
    if (showTangent_) {
        double min = 0.0, max = 1.0;
        ImGui::SliderScalar("Tangent Parameter", ImGuiDataType_Double, &tangentParam_, &min, &max);
    }

    // Point on curve parameter slider
    if (showPointOnCurve_) {
        double min = 0.0, max = 1.0;
        ImGui::SliderScalar("Point Parameter", ImGuiDataType_Double, &pointOnCurveParam_, &min, &max);
    }

    ImGui::Separator();

    // Control point editing
    ImGui::Text("Control Points:");
    PointVector2d points = curve_->controlPoints();

    for (size_t i = 0; i < points.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        ImGui::Text("Point %zu: (%.2f, %.2f)", i, points[i].x(), points[i].y());

        // Add sliders for editing
        float x = static_cast<float>(points[i].x());
        float y = static_cast<float>(points[i].y());

        if (ImGui::SliderFloat("X", &x, static_cast<float>(worldMinX_), static_cast<float>(worldMaxX_))) {
            points[i].x() = x;
            curve_->setControlPoints(points);
        }

        if (ImGui::SliderFloat("Y", &y, static_cast<float>(worldMinY_), static_cast<float>(worldMaxY_))) {
            points[i].y() = y;
            curve_->setControlPoints(points);
        }

        ImGui::PopID();
    }

    ImGui::Separator();

    // Add/Remove control points
    if (ImGui::Button("Add Control Point") && points.size() < 10) {
        // Add a new point at the end
        Point2d lastPoint = points.back();
        points.emplace_back(lastPoint.x() + 20, lastPoint.y());
        curve_->setControlPoints(points);
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove Control Point") && points.size() > 2) {
        points.pop_back();
        curve_->setControlPoints(points);
    }

    ImGui::Separator();

    // Reset button
    if (ImGui::Button("Reset to Default")) {
        PointVector2d defaultPoints;
        defaultPoints.emplace_back(-50, -20);
        defaultPoints.emplace_back(-30, 50);
        defaultPoints.emplace_back(30, 50);
        defaultPoints.emplace_back(50, -20);
        curve_->setControlPoints(defaultPoints);
    }

    ImGui::End();

    // Render info panel
    renderInfoPanel();
}

void BezierEditor::renderCanvas(const ImVec2& canvasPos) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (!drawList || !curve_) return;

    // Get control points
    PointVector2d points = curve_->controlPoints();

    // Convert control points to screen coordinates
    std::vector<ImVec2> screenPoints;
    for (const auto& pt : points) {
        float sx, sy;
        renderer_->convertWorldToScreen(pt, sx, sy);
        screenPoints.push_back(ImVec2(canvasPos.x + sx, canvasPos.y + sy));
    }

    // Draw control polygon
    if (showControlPolygon_) {
        for (size_t i = 0; i < screenPoints.size() - 1; ++i) {
            drawList->AddLine(screenPoints[i], screenPoints[i + 1],
                            IM_COL32(128, 128, 128, 128), 1.5f);
        }
    }

    // Draw Bezier curve
    const int segments = 100;
    for (int i = 0; i < segments; ++i) {
        double t1 = static_cast<double>(i) / segments;
        double t2 = static_cast<double>(i + 1) / segments;

        Point2d p1 = curve_->evaluate(t1);
        Point2d p2 = curve_->evaluate(t2);

        float sx1, sy1, sx2, sy2;
        renderer_->convertWorldToScreen(p1, sx1, sy1);
        renderer_->convertWorldToScreen(p2, sx2, sy2);

        drawList->AddLine(
            ImVec2(canvasPos.x + sx1, canvasPos.y + sy1),
            ImVec2(canvasPos.x + sx2, canvasPos.y + sy2),
            IM_COL32(255, 255, 255, 255), 2.0f
        );
    }

    // Draw control points
    if (showControlPoints_) {
        for (size_t i = 0; i < screenPoints.size(); ++i) {
            ImU32 color = (i == selectedControlPoint_) ?
                         IM_COL32(255, 255, 0, 255) :  // Yellow for selected
                         IM_COL32(255, 165, 0, 255);    // Orange for others

            drawList->AddCircleFilled(screenPoints[i], controlPointSize_, color);
            drawList->AddCircle(screenPoints[i], controlPointSize_, IM_COL32(255, 255, 255, 255));
        }
    }

    // Draw tangent
    if (showTangent_) {
        Point2d tangentPoint = curve_->evaluate(tangentParam_);
        Point2d tangentDir = curve_->derivative(tangentParam_);
        double tangentLength = 30.0;

        float tx, ty;
        renderer_->convertWorldToScreen(tangentPoint, tx, ty);

        // Normalize and scale tangent
        double norm = tangentDir.norm();
        if (norm > 1e-6) {
            Point2d tangentEnd = tangentPoint + (tangentDir / norm) * tangentLength;
            float ex, ey;
            renderer_->convertWorldToScreen(tangentEnd, ex, ey);

            drawList->AddLine(
                ImVec2(canvasPos.x + tx, canvasPos.y + ty),
                ImVec2(canvasPos.x + ex, canvasPos.y + ey),
                IM_COL32(0, 255, 0, 255), 2.0f
            );
        }
    }

    // Draw point on curve
    if (showPointOnCurve_) {
        Point2d pt = curve_->evaluate(pointOnCurveParam_);
        float px, py;
        renderer_->convertWorldToScreen(pt, px, py);

        drawList->AddCircleFilled(
            ImVec2(canvasPos.x + px, canvasPos.y + py),
            8.0f, IM_COL32(255, 0, 0, 255)
        );
    }
}

void BezierEditor::renderInfoPanel() {
    ImGui::Begin("Bezier Curve Info");

    // Curve information
    ImGui::Text("Degree: %d", curve_->degree());
    ImGui::Text("Number of Control Points: %zu", curve_->controlPoints().size());

    ImGui::Separator();

    // Curve properties at t = 0.5
    Point2d p05 = curve_->evaluate(0.5);
    Point2d tangent = curve_->derivative(0.5);
    double tangentNorm = tangent.norm();

    ImGui::Text("Point at t=0.5: (%.2f, %.2f)", p05.x(), p05.y());
    ImGui::Text("Tangent at t=0.5: (%.2f, %.2f)", tangent.x(), tangent.y());
    ImGui::Text("Tangent Norm: %.2f", tangentNorm);

    ImGui::Separator();

    // Instructions
    ImGui::Text("Instructions:");
    ImGui::BulletText("Drag control points to modify the curve");
    ImGui::BulletText("Use sliders to fine-tune positions");
    ImGui::BulletText("Toggle visualization options");

    ImGui::End();
}

void BezierEditor::handleMouseButton(int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Check if clicking on a control point
            double xpos, ypos;
            glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);

            int foundPoint = findControlPoint(xpos, ypos);
            if (foundPoint >= 0) {
                selectedControlPoint_ = foundPoint;
                isDragging_ = true;
            }
        } else if (action == GLFW_RELEASE) {
            isDragging_ = false;
            selectedControlPoint_ = -1;
        }
    }
}

void BezierEditor::handleMousePosition(double xpos, double ypos) {
    if (isDragging_ && selectedControlPoint_ >= 0) {
        auto* bezierRenderer = static_cast<BezierRenderer*>(renderer_.get());
        Point2d worldPos = bezierRenderer->convertScreenToWorld(xpos, ypos);
        PointVector2d points = curve_->controlPoints();
        points[selectedControlPoint_] = worldPos;
        curve_->setControlPoints(points);
    }
}

int BezierEditor::findControlPoint(double mouseX, double mouseY) {
    PointVector2d points = curve_->controlPoints();
    auto* bezierRenderer = static_cast<BezierRenderer*>(renderer_.get());

    for (size_t i = 0; i < points.size(); ++i) {
        float sx, sy;
        bezierRenderer->convertWorldToScreen(points[i], sx, sy);

        double dx = mouseX - sx;
        double dy = mouseY - sy;
        double dist = std::sqrt(dx * dx + dy * dy);

        if (dist < controlPointSize_ + 5) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

} // namespace cagd
