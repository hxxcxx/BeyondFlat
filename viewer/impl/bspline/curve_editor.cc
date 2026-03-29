#define NOMINMAX
#include "viewer/impl/bspline/curve_editor.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <sstream>
#include <windows.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cagd {

BSplineEditor::BSplineEditor()
    : CurveEditor()
    , showControlPoints_(true)
    , showControlPolygon_(true)
    , showKnotPoints_(false)
    , showTangent_(false)
    , showPointOnCurve_(false)
    , tangentParam_(0.5)
    , pointOnCurveParam_(0.5)
    , knotInsertParam_(0.5)
    , knotMultiplicity_(1)
    , showKnotInsertPoint_(false)
    , selectedControlPoint_(-1)
    , isDragging_(false)
    , controlPointSize_(10.0f)
    , degree_(3) {

    // Set screen dimensions in base class
    screenWidth_ = 1280;
    screenHeight_ = 720;

    // Set world bounds in base class
    worldMinX_ = -100;
    worldMaxX_ = 100;
    worldMinY_ = -100;
    worldMaxY_ = 100;

    // Initialize with default control points (cubic B-spline)
    PointVector2d controlPoints;
    controlPoints.emplace_back(-50, -20);
    controlPoints.emplace_back(-30, 50);
    controlPoints.emplace_back(30, 50);
    controlPoints.emplace_back(50, -20);

    curve_ = std::make_unique<BSplineCurve2d>(degree_, controlPoints);
}

void BSplineEditor::initialize() {
    // Nothing to initialize
}

void BSplineEditor::render() {
    renderControlPanel();

    // Render 2D canvas
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(330, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(displaySize.x - 330, displaySize.y), ImGuiCond_Always);
    ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    setScreenSize(static_cast<int>(canvasSize.x), static_cast<int>(canvasSize.y));

    ImGui::InvisibleButton("Canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    bool isHovered = ImGui::IsItemHovered();
    if (isHovered) {
        ImVec2 mousePos = ImGui::GetMousePos();
        double canvasMouseX = mousePos.x - canvasPos.x;
        double canvasMouseY = mousePos.y - canvasPos.y;

        if (ImGui::IsMouseClicked(0)) {
            handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0, canvasMouseX, canvasMouseY);
        } else if (ImGui::IsMouseReleased(0)) {
            handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0, canvasMouseX, canvasMouseY);
        }
        if (ImGui::IsMouseClicked(1)) {
            handleMouseButton(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0, canvasMouseX, canvasMouseY);
        } else if (ImGui::IsMouseReleased(1)) {
            handleMouseButton(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, 0, canvasMouseX, canvasMouseY);
        }
        if (ImGui::IsMouseDragging(0)) {
            handleMousePosition(canvasMouseX, canvasMouseY);
        }
    }

    renderCanvas(canvasPos);

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void BSplineEditor::renderControlPanel() {
    ImGui::SetNextWindowPos(ImVec2(0, 200), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(350, 520), ImGuiCond_Always);

    ImGui::Begin("B-Spline Curve Control Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // Display options
    ImGui::Checkbox("Show Control Points", &showControlPoints_);
    ImGui::Checkbox("Show Control Polygon", &showControlPolygon_);
    ImGui::Checkbox("Show Knot Points", &showKnotPoints_);
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

    // Knot insertion section
    ImGui::Text("Knot Insertion:");
    ImGui::Checkbox("Show Knot Insert Point", &showKnotInsertPoint_);

    if (showKnotInsertPoint_) {
        double min = 0.0, max = 1.0;
        ImGui::SliderScalar("Knot Parameter", ImGuiDataType_Double, &knotInsertParam_, &min, &max);
        ImGui::InputInt("Multiplicity", &knotMultiplicity_);
        knotMultiplicity_ = std::max(1, std::min(knotMultiplicity_, degree_));

        if (ImGui::Button("Insert Knot")) {
            performKnotInsertion();
        }
        ImGui::Text("(Press 'I' to insert knot)");
    }

    ImGui::Separator();

    // Reset button
    if (ImGui::Button("Reset to Default")) {
        PointVector2d defaultPoints;
        defaultPoints.emplace_back(-50, -20);
        defaultPoints.emplace_back(-30, 50);
        defaultPoints.emplace_back(30, 50);
        defaultPoints.emplace_back(50, -20);
        degree_ = 3;
        curve_ = std::make_unique<BSplineCurve2d>(degree_, defaultPoints);
    }

    ImGui::Separator();

    // Curve information
    ImGui::Text("Curve Info:");
    ImGui::Text("Degree: %d", curve_->degree());
    ImGui::Text("Control Points: %zu / 20", curve_->controlPoints().size());
    ImGui::Text("Knots: %zu", curve_->knotCount());

    // Display knot vector
    if (ImGui::CollapsingHeader("Knot Vector")) {
        const auto& knots = curve_->knots();
        std::string knotStr;
        for (size_t i = 0; i < knots.size(); ++i) {
            knotStr += std::to_string(knots[i]) + " ";
            if ((i + 1) % 8 == 0) knotStr += "\n";
        }
        ImGui::Text("%s", knotStr.c_str());
    }

    ImGui::Separator();

    // Curve properties at t = 0.5
    auto domain = curve_->domain();
    Point2d p05 = curve_->evaluate(0.5);
    Point2d tangent = curve_->derivative(0.5);
    double tangentNorm = tangent.norm();

    ImGui::Text("Domain: [%.3f, %.3f]", domain.first, domain.second);
    ImGui::Text("Point at t=0.5: (%.2f, %.2f)", p05.x(), p05.y());
    ImGui::Text("Tangent at t=0.5: (%.2f, %.2f)", tangent.x(), tangent.y());
    ImGui::Text("Tangent Norm: %.2f", tangentNorm);

    ImGui::Separator();

    // Instructions
    ImGui::Text("Instructions:");
    ImGui::BulletText("Left-click on empty space to add control point");
    ImGui::BulletText("Left-click and drag to move control points");
    ImGui::BulletText("Right-click on control point to delete it");
    ImGui::BulletText("Press 'I' to insert knot at selected parameter");
    ImGui::BulletText("Use sliders to fine-tune positions");
    ImGui::BulletText("Toggle visualization options");

    ImGui::End();
}

void BSplineEditor::renderCanvas(const ImVec2& canvasPos) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (!drawList || !curve_) return;

    // Get control points
    PointVector2d points = curve_->controlPoints();

    // Debug: log control points and knots
    {
        std::ostringstream oss;
        const auto& knots = curve_->knots();
        oss << "=== " << points.size() << " pts, deg " << curve_->degree() << ", " << knots.size() << " knots ===\n";
        oss << "Knots: ";
        for (size_t i = 0; i < knots.size(); ++i) oss << knots[i] << " ";
        oss << "\n";
        for (size_t i = 0; i < points.size(); ++i) {
            oss << "P" << i << "(" << points[i].x() << "," << points[i].y() << ") ";
        }
        oss << "\n";
        OutputDebugStringA(oss.str().c_str());
    }

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

    // Debug: log first few curve evaluations
    {
        std::ostringstream oss;
        oss << "Curve points at t=0, 0.25, 0.5, 0.75, 1:\n";
        for (double t_val : {0.0, 0.25, 0.5, 0.75, 1.0}) {
            Point2d p = curve_->evaluate(t_val);
            oss << "  t=" << t_val << " -> (" << p.x() << ", " << p.y() << ")\n";
        }
        OutputDebugStringA(oss.str().c_str());
    }

    // Draw B-spline curve
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

    // Draw knot points
    if (showKnotPoints_) {
        const auto& knots = curve_->knots();
        // Show interior knots (not clamped endpoints)
        for (size_t i = curve_->degree(); i < knots.size() - curve_->degree() - 1; ++i) {
            double t = knots[i];
            Point2d knotPt = curve_->evaluate(t);
            float kx, ky;
            worldToScreen(knotPt, kx, ky);

            drawList->AddCircleFilled(
                ImVec2(canvasPos.x + kx, canvasPos.y + ky),
                5.0f, IM_COL32(255, 255, 0, 255)
            );
        }
    }

    // Draw control points
    if (showControlPoints_) {
        for (size_t i = 0; i < screenPoints.size(); ++i) {
            ImU32 color = (i == selectedControlPoint_) ?
                         IM_COL32(255, 255, 0, 255) :  // Yellow for selected
                         IM_COL32(0, 200, 255, 255);    // Cyan for others (B-spline specific)

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

    // Draw knot insertion point
    if (showKnotInsertPoint_) {
        Point2d pt = curve_->evaluate(knotInsertParam_);
        float px, py;
        worldToScreen(pt, px, py);

        // Draw knot insertion point with magenta color
        drawList->AddCircleFilled(
            ImVec2(canvasPos.x + px, canvasPos.y + py),
            10.0f, IM_COL32(255, 0, 255, 255)
        );
        drawList->AddCircle(
            ImVec2(canvasPos.x + px, canvasPos.y + py),
            10.0f, IM_COL32(255, 255, 255, 255), 2.0f
        );
    }
}

void BSplineEditor::handleMouseButton(int button, int action, int, double xpos, double ypos) {
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
                OutputDebugStringA(("BSplineEditor: Click at screen(" + std::to_string(xpos) + ", " + std::to_string(ypos) + ") -> world(" + std::to_string(worldPos.x()) + ", " + std::to_string(worldPos.y()) + ")\n").c_str());

                PointVector2d points = curve_->controlPoints();
                OutputDebugStringA(("BSplineEditor: Before add, control points count = " + std::to_string(points.size()) + "\n").c_str());

                // Limit maximum number of control points
                if (points.size() < 20) {
                    points.push_back(worldPos);
                    curve_->setControlPoints(points);

                    OutputDebugStringA(("BSplineEditor: After add, control points count = " + std::to_string(curve_->controlPoints().size()) + "\n").c_str());
                    OutputDebugStringA(("BSplineEditor: Knot count = " + std::to_string(curve_->knotCount()) + ", degree = " + std::to_string(curve_->degree()) + "\n").c_str());

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

                // Keep minimum number of control points (degree + 1)
                int minPoints = curve_->degree() + 1;
                if (points.size() > minPoints) {
                    points.erase(points.begin() + foundPoint);
                    curve_->setControlPoints(points);
                    selectedControlPoint_ = -1;
                }
            }
        }
    }
}

void BSplineEditor::handleMousePosition(double xpos, double ypos) {
    if (isDragging_ && selectedControlPoint_ >= 0) {
        Point2d worldPos = screenToWorld(xpos, ypos);
        PointVector2d points = curve_->controlPoints();
        points[selectedControlPoint_] = worldPos;
        curve_->setControlPoints(points);
    }
}

int BSplineEditor::findControlPoint(double mouseX, double mouseY) {
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

void BSplineEditor::performKnotInsertion() {
    if (!curve_) return;

    // Insert knot at the current parameter
    curve_->insertKnot(knotInsertParam_, knotMultiplicity_);
}

void BSplineEditor::handleKey(int key, int action, int mods) {
    // Only handle key press events
    if (action != GLFW_PRESS) return;

    // Knot insertion on 'I' key
    if (key == GLFW_KEY_I && !mods) {
        if (showKnotInsertPoint_) {
            performKnotInsertion();
        }
    }
}

} // namespace cagd
