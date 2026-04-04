#define NOMINMAX
#include "viewer/impl/nurbs/curve_editor.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <sstream>
#include <windows.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cagd {

NURBSEditor::NURBSEditor()
    : CurveEditor()
    , showControlPoints_(true)
    , showControlPolygon_(true)
    , showKnotPoints_(false)
    , showTangent_(false)
    , showPointOnCurve_(false)
    , showWeightedInfluence_(false)
    , tangentParam_(0.5)
    , pointOnCurveParam_(0.5)
    , knotInsertParam_(0.5)
    , knotMultiplicity_(1)
    , showKnotInsertPoint_(false)
    , selectedControlPoint_(-1)
    , isDragging_(false)
    , controlPointSize_(10.0f)
    , degree_(2) {

    screenWidth_ = 1280;
    screenHeight_ = 720;

    worldMinX_ = -200;
    worldMaxX_ = 200;
    worldMinY_ = -200;
    worldMaxY_ = 200;

    // Initialize with a quarter circle as default NURBS example
    createCircleExample();
}

void NURBSEditor::initialize() {
}

void NURBSEditor::createCircleExample() {
    // NURBS representation of a unit circle (using 9 control points, degree 2)
    // This is a well-known NURBS circle construction
    degree_ = 2;

    PointVector2d controlPoints;
    std::vector<double> weights;
    std::vector<double> knots;

    double r = 80.0; // radius in world units
    double sq2 = std::sqrt(2.0) / 2.0;

    // Quarter circle 1: 0-90 degrees
    controlPoints.emplace_back(r, 0);
    controlPoints.emplace_back(r, r);
    controlPoints.emplace_back(0, r);

    // Quarter circle 2: 90-180 degrees
    controlPoints.emplace_back(-r, r);
    controlPoints.emplace_back(-r, 0);

    // Quarter circle 3: 180-270 degrees
    controlPoints.emplace_back(-r, -r);
    controlPoints.emplace_back(0, -r);

    // Quarter circle 4: 270-360 degrees
    controlPoints.emplace_back(r, -r);
    controlPoints.emplace_back(r, 0);

    double w = sq2; // cos(pi/4)
    weights = {1.0, w, 1.0, w, 1.0, w, 1.0, w, 1.0};

    // Clamped knot vector for degree 2 with 9 control points
    // n=8, p=2, knots = n+p+2 = 12
    knots = {0, 0, 0, 0.25, 0.25, 0.5, 0.5, 0.75, 0.75, 1, 1, 1};

    curve_ = std::make_unique<NURBSCurve2d>(degree_, controlPoints, weights, knots);
}

void NURBSEditor::createEllipseExample() {
    degree_ = 2;

    PointVector2d controlPoints;
    std::vector<double> weights;
    std::vector<double> knots;

    double a = 120.0; // semi-major axis
    double b = 60.0; // semi-minor axis
    double sq2 = std::sqrt(2.0) / 2.0;

    // Ellipse using NURBS (similar to circle but with different radii)
    controlPoints.emplace_back(a, 0);
    controlPoints.emplace_back(a, b);
    controlPoints.emplace_back(0, b);
    controlPoints.emplace_back(-a, b);
    controlPoints.emplace_back(-a, 0);
    controlPoints.emplace_back(-a, -b);
    controlPoints.emplace_back(0, -b);
    controlPoints.emplace_back(a, -b);
    controlPoints.emplace_back(a, 0);

    double w = sq2;
    weights = {1.0, w, 1.0, w, 1.0, w, 1.0, w, 1.0};

    knots = {0, 0, 0, 0.25, 0.25, 0.5, 0.5, 0.75, 0.75, 1, 1, 1};

    curve_ = std::make_unique<NURBSCurve2d>(degree_, controlPoints, weights, knots);
}

void NURBSEditor::createArcExample() {
    // 90-degree arc using 3 control points, degree 2
    degree_ = 2;

    PointVector2d controlPoints;
    std::vector<double> weights;

    double r = 100.0;
    double sq2 = std::sqrt(2.0) / 2.0;

    controlPoints.emplace_back(r, 0);
    controlPoints.emplace_back(r * sq2, r * (1 + sq2));  // shoulder point for 90° arc
    controlPoints.emplace_back(0, r);

    weights = {1.0, sq2, 1.0};

    std::vector<double> knots = {0, 0, 0, 1, 1, 1};

    curve_ = std::make_unique<NURBSCurve2d>(degree_, controlPoints, weights, knots);
}

void NURBSEditor::updateAspectScale() {
    // Adjust world bounds to preserve aspect ratio (uniform scale in both axes)
    double worldW = worldMaxX_ - worldMinX_;
    double worldH = worldMaxY_ - worldMinY_;
    double aspectScreen = static_cast<double>(screenWidth_) / static_cast<double>(screenHeight_);
    double aspectWorld = worldW / worldH;

    double centerX = (worldMinX_ + worldMaxX_) / 2.0;
    double centerY = (worldMinY_ + worldMaxY_) / 2.0;

    if (aspectScreen > aspectWorld) {
        // Screen is wider — expand world X range
        double newWorldW = worldH * aspectScreen;
        worldMinX_ = centerX - newWorldW / 2.0;
        worldMaxX_ = centerX + newWorldW / 2.0;
    } else {
        // Screen is taller — expand world Y range
        double newWorldH = worldW / aspectScreen;
        worldMinY_ = centerY - newWorldH / 2.0;
        worldMaxY_ = centerY + newWorldH / 2.0;
    }
}

void NURBSEditor::worldToScreenAR(const Point2d& world, float& screenX, float& screenY) {
    screenX = static_cast<float>((world.x() - worldMinX_) / (worldMaxX_ - worldMinX_) * screenWidth_);
    screenY = static_cast<float>((world.y() - worldMinY_) / (worldMaxY_ - worldMinY_) * screenHeight_);
}

Point2d NURBSEditor::screenToWorldAR(double screenX, double screenY) {
    double worldX = worldMinX_ + (screenX / screenWidth_) * (worldMaxX_ - worldMinX_);
    double worldY = worldMinY_ + (screenY / screenHeight_) * (worldMaxY_ - worldMinY_);
    return Point2d(worldX, worldY);
}

void NURBSEditor::render() {
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
    updateAspectScale();

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

void NURBSEditor::renderControlPanel() {
    ImGui::SetNextWindowPos(ImVec2(0, 200), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(350, 520), ImGuiCond_Always);

    ImGui::Begin("NURBS Curve Control Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // Display options
    ImGui::Checkbox("Show Control Points", &showControlPoints_);
    ImGui::Checkbox("Show Control Polygon", &showControlPolygon_);
    ImGui::Checkbox("Show Knot Points", &showKnotPoints_);
    ImGui::Checkbox("Show Tangent", &showTangent_);
    ImGui::Checkbox("Show Point on Curve", &showPointOnCurve_);
    ImGui::Checkbox("Show Weight Influence", &showWeightedInfluence_);

    ImGui::Separator();

    // Preset curves
    ImGui::Text("Preset Curves:");
    if (ImGui::Button("Circle")) {
        createCircleExample();
    }
    ImGui::SameLine();
    if (ImGui::Button("Ellipse")) {
        createEllipseExample();
    }
    ImGui::SameLine();
    if (ImGui::Button("Arc")) {
        createArcExample();
    }

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

    // Weight editing
    ImGui::Text("Control Point Weights:");
    {
        auto points = curve_->controlPoints();
        auto weights = curve_->weights();
        int count = std::min(static_cast<int>(points.size()), 20);
        for (int i = 0; i < count; ++i) {
            ImGui::PushID(i);
            float w = static_cast<float>(weights[i]);
            if (ImGui::SliderFloat("W", &w, 0.01f, 5.0f, "%.2f")) {
                weights[i] = static_cast<double>(w);
                curve_->setWeights(weights);
            }
            ImGui::SameLine();
            ImGui::Text("P%d", i);
            ImGui::PopID();
        }
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
    }

    ImGui::Separator();

    // Reset / clear
    if (ImGui::Button("Reset to Default")) {
        createCircleExample();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        PointVector2d emptyPoints;
        emptyPoints.emplace_back(0, 0);
        emptyPoints.emplace_back(0, 0);
        emptyPoints.emplace_back(0, 0);
        degree_ = 2;
        std::vector<double> w = {1.0, 1.0, 1.0};
        std::vector<double> k = {0, 0, 0, 1, 1, 1};
        curve_ = std::make_unique<NURBSCurve2d>(degree_, emptyPoints, w, k);
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

    // Display weights
    if (ImGui::CollapsingHeader("Weights")) {
        const auto& w = curve_->weights();
        std::string wStr;
        for (size_t i = 0; i < w.size(); ++i) {
            wStr += std::to_string(w[i]) + " ";
            if ((i + 1) % 8 == 0) wStr += "\n";
        }
        ImGui::Text("%s", wStr.c_str());
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
    ImGui::BulletText("Left-click empty space to add control point");
    ImGui::BulletText("Left-click and drag to move control points");
    ImGui::BulletText("Right-click on control point to delete it");
    ImGui::BulletText("Adjust weights with sliders above");
    ImGui::BulletText("Use presets to load conic sections");
    ImGui::BulletText("Toggle visualization options");

    ImGui::End();
}

void NURBSEditor::renderCanvas(const ImVec2& canvasPos) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (!drawList || !curve_) return;

    PointVector2d points = curve_->controlPoints();
    const auto& weights = curve_->weights();

    // Convert control points to screen coordinates
    std::vector<ImVec2> screenPoints;
    for (const auto& pt : points) {
        float sx, sy;
        worldToScreenAR(pt, sx, sy);
        screenPoints.push_back(ImVec2(canvasPos.x + sx, canvasPos.y + sy));
    }

    // Draw weighted influence (larger circles for higher weights)
    if (showWeightedInfluence_) {
        for (size_t i = 0; i < screenPoints.size(); ++i) {
            float influence = static_cast<float>(weights[i]) * 15.0f;
            drawList->AddCircle(screenPoints[i], influence,
                              IM_COL32(255, 165, 0, 80), 32, 1.5f);
        }
    }

    // Draw control polygon
    if (showControlPolygon_) {
        for (size_t i = 0; i < screenPoints.size() - 1; ++i) {
            drawList->AddLine(screenPoints[i], screenPoints[i + 1],
                            IM_COL32(128, 128, 128, 128), 1.5f);
        }
    }

    // Draw NURBS curve
    const int segments = 200;
    for (int i = 0; i < segments; ++i) {
        double t1 = static_cast<double>(i) / segments;
        double t2 = static_cast<double>(i + 1) / segments;

        Point2d p1 = curve_->evaluate(t1);
        Point2d p2 = curve_->evaluate(t2);

        float sx1, sy1, sx2, sy2;
        worldToScreenAR(p1, sx1, sy1);
        worldToScreenAR(p2, sx2, sy2);

        drawList->AddLine(
            ImVec2(canvasPos.x + sx1, canvasPos.y + sy1),
            ImVec2(canvasPos.x + sx2, canvasPos.y + sy2),
            IM_COL32(100, 200, 255, 255), 2.5f
        );
    }

    // Draw knot points
    if (showKnotPoints_) {
        const auto& knots = curve_->knots();
        for (size_t i = curve_->degree(); i < knots.size() - curve_->degree() - 1; ++i) {
            double t = knots[i];
            Point2d knotPt = curve_->evaluate(t);
            float kx, ky;
            worldToScreenAR(knotPt, kx, ky);

            drawList->AddCircleFilled(
                ImVec2(canvasPos.x + kx, canvasPos.y + ky),
                5.0f, IM_COL32(255, 255, 0, 255)
            );
        }
    }

    // Draw control points (color coded by weight)
    if (showControlPoints_) {
        for (size_t i = 0; i < screenPoints.size(); ++i) {
            // Color based on weight: blue (low) -> green (1.0) -> red (high)
            float w = static_cast<float>(weights[i]);
            int r = static_cast<int>(std::min(255.0, std::max(0.0, (w - 1.0) * 255.0)));
            int gb = static_cast<int>(255 - std::abs(w - 1.0) * 200);
            gb = std::max(0, std::min(255, gb));
            int b = static_cast<int>(std::min(255.0, std::max(0.0, (1.0 - w) * 255.0)));

            ImU32 color = (i == static_cast<size_t>(selectedControlPoint_)) ?
                         IM_COL32(255, 255, 0, 255) :
                         IM_COL32(r, gb, b, 255);

            float size = controlPointSize_ * (0.5f + 0.5f * w);
            drawList->AddCircleFilled(screenPoints[i], size, color);
            drawList->AddCircle(screenPoints[i], size, IM_COL32(255, 255, 255, 255));

            // Draw weight label
            char label[16];
            snprintf(label, sizeof(label), "%.1f", weights[i]);
            drawList->AddText(
                ImVec2(screenPoints[i].x + size + 2, screenPoints[i].y - 8),
                IM_COL32(200, 200, 200, 200), label
            );
        }
    }

    // Draw tangent
    if (showTangent_) {
        Point2d tangentPoint = curve_->evaluate(tangentParam_);
        Point2d tangentDir = curve_->derivative(tangentParam_);
        double tangentLength = 30.0;

        float tx, ty;
        worldToScreenAR(tangentPoint, tx, ty);

        double norm = tangentDir.norm();
        if (norm > 1e-6) {
            Point2d tangentEnd = tangentPoint + (tangentDir / norm) * tangentLength;
            float ex, ey;
            worldToScreenAR(tangentEnd, ex, ey);

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
        worldToScreenAR(pt, px, py);

        drawList->AddCircleFilled(
            ImVec2(canvasPos.x + px, canvasPos.y + py),
            8.0f, IM_COL32(255, 0, 0, 255)
        );
    }

    // Draw knot insertion point
    if (showKnotInsertPoint_) {
        Point2d pt = curve_->evaluate(knotInsertParam_);
        float px, py;
        worldToScreenAR(pt, px, py);

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

void NURBSEditor::handleMouseButton(int button, int action, int, double xpos, double ypos) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            int foundPoint = findControlPoint(xpos, ypos);
            if (foundPoint >= 0) {
                selectedControlPoint_ = foundPoint;
                isDragging_ = true;
            } else {
                // Add new control point with default weight 1.0
                Point2d worldPos = screenToWorldAR(xpos, ypos);

                PointVector2d points = curve_->controlPoints();
                std::vector<double> weights = curve_->weights();

                if (points.size() < 20) {
                    points.push_back(worldPos);
                    weights.push_back(1.0);
                    curve_->setControlPoints(points);
                    curve_->setWeights(weights);

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
            int foundPoint = findControlPoint(xpos, ypos);
            if (foundPoint >= 0) {
                PointVector2d points = curve_->controlPoints();
                std::vector<double> weights = curve_->weights();

                int minPoints = curve_->degree() + 1;
                if (static_cast<int>(points.size()) > minPoints) {
                    points.erase(points.begin() + foundPoint);
                    weights.erase(weights.begin() + foundPoint);
                    curve_->setControlPoints(points);
                    curve_->setWeights(weights);
                    selectedControlPoint_ = -1;
                }
            }
        }
    }
}

void NURBSEditor::handleMousePosition(double xpos, double ypos) {
    if (isDragging_ && selectedControlPoint_ >= 0) {
        Point2d worldPos = screenToWorldAR(xpos, ypos);
        PointVector2d points = curve_->controlPoints();
        points[selectedControlPoint_] = worldPos;
        curve_->setControlPoints(points);
    }
}

int NURBSEditor::findControlPoint(double mouseX, double mouseY) {
    PointVector2d points = curve_->controlPoints();
    const auto& weights = curve_->weights();

    for (size_t i = 0; i < points.size(); ++i) {
        float sx, sy;
        worldToScreenAR(points[i], sx, sy);

        double dx = mouseX - sx;
        double dy = mouseY - sy;
        double dist = std::sqrt(dx * dx + dy * dy);

        // Larger hit area for heavier points
        float hitSize = controlPointSize_ * (0.5f + 0.5f * static_cast<float>(weights[i]));
        if (dist < hitSize + 5) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void NURBSEditor::performKnotInsertion() {
    if (!curve_) return;
    curve_->insertKnot(knotInsertParam_, knotMultiplicity_);
}

void NURBSEditor::handleKey(int key, int action, int mods) {
    if (action != GLFW_PRESS) return;

    (void)key;
    (void)mods;
}

} // namespace cagd
