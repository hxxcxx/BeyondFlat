#include "bezier_editor.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cagd {

BezierEditor::BezierEditor()
    : showControlPoints_(true)
    , showControlPolygon_(true)
    , showTangent_(false)
    , showDeCasteljau_(false)
    , tangentParam_(0.5)
    , deCasteljauParam_(0.5)
    , selectedControlPoint_(-1)
    , isDragging_(false)
    , screenWidth_(1280)
    , screenHeight_(720)
    , worldMinX_(-100), worldMaxX_(100)
    , worldMinY_(-100), worldMaxY_(100)
    , controlPointSize_(10.0f) {

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
}

void BezierEditor::render() {
    // Render control panel
    renderControlPanel();

    // Render info panel
    renderInfoPanel();

    // Render the curve and related elements
    if (showControlPolygon_) {
        renderer_->renderControlPolygon(curve_->controlPoints());
    }

    renderer_->renderCurve(*curve_, 100);

    if (showControlPoints_) {
        renderer_->renderControlPoints(curve_->controlPoints(), controlPointSize_);
    }

    if (showTangent_) {
        renderer_->renderTangent(*curve_, tangentParam_, 30.0f);
        renderer_->renderPointOnCurve(*curve_, tangentParam_, 8.0f, 1.0f, 0.0f, 0.0f);
    }

    if (showDeCasteljau_) {
        renderDeCasteljauVisualization();
    }
}

void BezierEditor::renderControlPanel() {
    ImGui::Begin("Bezier Curve Control Panel");

    // Display options
    ImGui::Checkbox("Show Control Points", &showControlPoints_);
    ImGui::Checkbox("Show Control Polygon", &showControlPolygon_);
    ImGui::Checkbox("Show Tangent", &showTangent_);
    ImGui::Checkbox("Show De Casteljau", &showDeCasteljau_);

    ImGui::Separator();

    // Tangent parameter slider
    if (showTangent_) {
        double min = 0.0, max = 1.0;
        ImGui::SliderScalar("Tangent Parameter", ImGuiDataType_Double, &tangentParam_, &min, &max);
    }

    // De Casteljau parameter slider
    if (showDeCasteljau_) {
        double min = 0.0, max = 1.0;
        ImGui::SliderScalar("De Casteljau Parameter", ImGuiDataType_Double, &deCasteljauParam_, &min, &max);
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

        if (ImGui::SliderFloat("X", &x, worldMinX_, worldMaxX_)) {
            points[i].x() = x;
            curve_->setControlPoints(points);
        }

        if (ImGui::SliderFloat("Y", &y, worldMinY_, worldMaxY_)) {
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

void BezierEditor::renderDeCasteljauVisualization() {
    // Visualize the De Casteljau algorithm
    renderer_->renderPointOnCurve(*curve_, deCasteljauParam_, 12.0f, 1.0f, 0.0f, 1.0f);

    // TODO: Implement full De Casteljau visualization
    // This would show intermediate points and lines
}

void BezierEditor::handleMouseButton(int button, int action, int mods) {
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
        Point2d worldPos = screenToWorld(xpos, ypos);
        PointVector2d points = curve_->controlPoints();
        points[selectedControlPoint_] = worldPos;
        curve_->setControlPoints(points);
    }
}

int BezierEditor::findControlPoint(double mouseX, double mouseY) {
    PointVector2d points = curve_->controlPoints();

    for (size_t i = 0; i < points.size(); ++i) {
        float sx, sy;
        renderer_->worldToScreen(points[i], sx, sy);

        double dx = mouseX - sx;
        double dy = mouseY - sy;
        double dist = std::sqrt(dx * dx + dy * dy);

        if (dist < controlPointSize_ + 5) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

Point2d BezierEditor::screenToWorld(double screenX, double screenY) {
    double worldX = worldMinX_ + (screenX / screenWidth_) * (worldMaxX_ - worldMinX_);
    double worldY = worldMinY_ + (screenY / screenHeight_) * (worldMaxY_ - worldMinY_);
    return Point2d(worldX, worldY);
}

void BezierEditor::setScreenSize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    renderer_->setScreenSize(width, height);
}

} // namespace cagd
