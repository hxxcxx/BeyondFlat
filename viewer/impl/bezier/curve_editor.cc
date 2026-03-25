#include "viewer/impl/bezier/curve_editor.h"
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
    , subdivideParam_(0.5)
    , showSubdividePoint_(false)
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
}

void BezierEditor::initialize() {
    // Nothing to initialize - we don't use renderer anymore
}

void BezierEditor::render() {
    // Nothing to render here - all rendering is done in renderCanvas()
}

void BezierEditor::renderControlPanel() {
    ImGui::SetNextWindowPos(ImVec2(0, 200), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(350, 520), ImGuiCond_Always);

    ImGui::Begin("Bezier Curve Control Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

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

    // Subdivision section
    ImGui::Checkbox("Show Subdivide Point", &showSubdividePoint_);
    if (showSubdividePoint_) {
        double min = 0.0, max = 1.0;
        ImGui::SliderScalar("Subdivide Parameter", ImGuiDataType_Double, &subdivideParam_, &min, &max);

        if (ImGui::Button("Subdivide at Parameter")) {
            performSubdivision();
        }

        ImGui::SameLine();
        ImGui::Text("(Press 'S' to subdivide)");
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

    ImGui::Separator();

    // Curve information
    ImGui::Text("Curve Info:");
    ImGui::Text("Degree: %d", curve_->degree());
    ImGui::Text("Control Points: %zu / 20", curve_->controlPoints().size());

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
    ImGui::BulletText("Left-click on empty space to add control point");
    ImGui::BulletText("Left-click and drag to move control points");
    ImGui::BulletText("Right-click on control point to delete it");
    ImGui::BulletText("Press 'S' to subdivide curve at selected point");
    ImGui::BulletText("Use sliders to fine-tune positions");
    ImGui::BulletText("Toggle visualization options");

    ImGui::End();
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
        worldToScreen(pt, sx, sy);
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
        worldToScreen(p1, sx1, sy1);
        worldToScreen(p2, sx2, sy2);

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
        worldToScreen(tangentPoint, tx, ty);

        // Normalize and scale tangent
        double norm = tangentDir.norm();
        if (norm > 1e-6) {
            Point2d tangentEnd = tangentPoint + (tangentDir / norm) * tangentLength;
            float ex, ey;
            worldToScreen(tangentEnd, ex, ey);

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
        worldToScreen(pt, px, py);

        drawList->AddCircleFilled(
            ImVec2(canvasPos.x + px, canvasPos.y + py),
            8.0f, IM_COL32(255, 0, 0, 255)
        );
    }

    // Draw subdivision point
    if (showSubdividePoint_) {
        Point2d pt = curve_->evaluate(subdivideParam_);
        float px, py;
        worldToScreen(pt, px, py);

        // Draw subdivision point with a different color (cyan)
        drawList->AddCircleFilled(
            ImVec2(canvasPos.x + px, canvasPos.y + py),
            10.0f, IM_COL32(0, 255, 255, 255)
        );
        drawList->AddCircle(
            ImVec2(canvasPos.x + px, canvasPos.y + py),
            10.0f, IM_COL32(255, 255, 255, 255), 2.0f
        );
    }
}

void BezierEditor::handleMouseButton(int button, int action, int, double xpos, double ypos) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Check if clicking on a control point (using canvas-relative coordinates)
            int foundPoint = findControlPoint(xpos, ypos);
            if (foundPoint >= 0) {
                // Clicked on existing control point, start dragging
                selectedControlPoint_ = foundPoint;
                isDragging_ = true;
            } else {
                // Clicked on empty space, add new control point
                Point2d worldPos = screenToWorld(xpos, ypos);

                PointVector2d points = curve_->controlPoints();

                // Limit maximum number of control points
                if (points.size() < 20) {
                    points.push_back(worldPos);
                    curve_->setControlPoints(points);

                    // Select the newly added point
                    selectedControlPoint_ = static_cast<int>(points.size()) - 1;
                    isDragging_ = true;
                }
            }
        } else if (action == GLFW_RELEASE) {
            isDragging_ = false;
            selectedControlPoint_ = -1;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // Check if right-clicking on a control point to delete it
            int foundPoint = findControlPoint(xpos, ypos);
            if (foundPoint >= 0) {
                // Delete the control point
                PointVector2d points = curve_->controlPoints();

                // Keep at least 2 control points
                if (points.size() > 2) {
                    points.erase(points.begin() + foundPoint);
                    curve_->setControlPoints(points);
                    selectedControlPoint_ = -1;
                }
            }
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
        worldToScreen(points[i], sx, sy);

        double dx = mouseX - sx;
        double dy = mouseY - sy;
        double dist = std::sqrt(dx * dx + dy * dy);

        if (dist < controlPointSize_ + 5) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void BezierEditor::performSubdivision() {
    if (!curve_) return;

    // Subdivide the curve at the current parameter
    auto [leftCurve, rightCurve] = curve_->subdivide(subdivideParam_);

    // Replace current curve with the left part
    // Note: In a more complete implementation, you might want to keep both curves
    // or add them to a list of curves. For now, we'll just use the left curve.
    curve_ = std::make_unique<BezierCurve2d>(leftCurve.controlPoints());

    // Reset subdivision point display
    showSubdividePoint_ = false;
}

void BezierEditor::handleKey(int key, int action, int mods) {
    // Only handle key press events
    if (action != GLFW_PRESS) return;

    // Subdivide on 'S' key
    if (key == GLFW_KEY_S && !mods) {
        if (showSubdividePoint_) {
            performSubdivision();
        }
    }
}

} // namespace cagd
