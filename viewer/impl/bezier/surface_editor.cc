#include "viewer/impl/bezier/surface_editor.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

namespace cagd {

SurfaceEditor::SurfaceEditor()
    : meshDirty_(true)
    , showSurface_(true)
    , showControlNet_(true)
    , showControlPoints_(true)
    , showAxes_(true)
    , showGrid_(true)
    , wireframeMode_(false)
    , resolutionU_(32)
    , resolutionV_(32)
    , selectedPoint_(-1, -1)
    , isDraggingPoint_(false)
    , pickRadius_(12.0)
    , editAxis_(0) {
}

void SurfaceEditor::initialize() {
    viewport_ = std::make_unique<Viewport3D>();
    resetToDefault();
}

void SurfaceEditor::render() {
    renderControlPanel();

    // Render into the same "Canvas" window as 2D editors
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(330, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(displaySize.x - 330, displaySize.y), ImGuiCond_Always);
    ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    renderViewport();

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void SurfaceEditor::renderControlPanel() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(330, 720), ImGuiCond_Always);

    ImGui::Begin("Surface Control Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // Visualization options
    ImGui::Text("Display");
    ImGui::Checkbox("Show Surface", &showSurface_);
    ImGui::Checkbox("Wireframe", &wireframeMode_);
    ImGui::Checkbox("Show Control Net", &showControlNet_);
    ImGui::Checkbox("Show Control Points", &showControlPoints_);
    ImGui::Checkbox("Show Axes", &showAxes_);
    ImGui::Checkbox("Show Grid", &showGrid_);

    ImGui::Separator();

    // Surface resolution
    ImGui::Text("Mesh Resolution");
    if (ImGui::SliderInt("U", &resolutionU_, 4, 64)) meshDirty_ = true;
    if (ImGui::SliderInt("V", &resolutionV_, 4, 64)) meshDirty_ = true;

    ImGui::Separator();

    // Surface info
    if (surface_) {
        ImGui::Text("Surface Info");
        ImGui::Text("Degree: %d x %d", surface_->degreeU(), surface_->degreeV());
        ImGui::Text("Control Points: %d x %d", surface_->numRows(), surface_->numCols());

        // Selected point info
        if (selectedPoint_.first >= 0 && selectedPoint_.second >= 0) {
            ImGui::Separator();
            ImGui::Text("Selected Point [%d, %d]:", selectedPoint_.first, selectedPoint_.second);
            Point3d p = surface_->controlPoint(selectedPoint_.first, selectedPoint_.second);
            ImGui::Text("Pos: (%.2f, %.2f, %.2f)", p.x(), p.y(), p.z());

            // Edit axis mode
            ImGui::Text("Drag Axis:");
            ImGui::RadioButton("XZ", &editAxis_, 0); ImGui::SameLine();
            ImGui::RadioButton("XY", &editAxis_, 1); ImGui::SameLine();
            ImGui::RadioButton("YZ", &editAxis_, 2); ImGui::SameLine();
            ImGui::RadioButton("Free", &editAxis_, 3);
        }
    }

    ImGui::Separator();

    // Degree operations
    if (surface_) {
        ImGui::Text("Degree Operations");
        if (ImGui::Button("Elevate Degree U")) {
            *surface_ = surface_->elevateDegreeU();
            meshDirty_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Elevate Degree V")) {
            *surface_ = surface_->elevateDegreeV();
            meshDirty_ = true;
        }
    }

    ImGui::Separator();

    // Reset
    if (ImGui::Button("Reset to Default")) {
        resetToDefault();
    }

    ImGui::Separator();

    // Instructions
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Controls:");
    ImGui::BulletText("Left-drag: Rotate camera");
    ImGui::BulletText("Right/Middle-drag: Pan camera");
    ImGui::BulletText("Scroll: Zoom");
    ImGui::BulletText("Left-click point: Select");
    ImGui::BulletText("Left-drag point: Move");
    ImGui::BulletText("Select axis mode for drag plane");

    ImGui::End();
}

void SurfaceEditor::renderViewport() {
    if (!viewport_) return;

    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 100 || canvasSize.y < 100) return;

    viewport_->begin("##viewport3d", canvasSize);
    renderScene();

    // Input handling only when hovered
    if (viewport_->isHovered()) {

        // Handle point picking on mouse click
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !isDraggingPoint_) {
            ImVec2 mousePos = ImGui::GetMousePos();
            auto vpRect = viewport_->rect();
            double localX = mousePos.x - vpRect.x;
            double localY = mousePos.y - vpRect.y;

            auto picked = pickControlPoint(localX, localY);
            if (picked.first >= 0) {
                selectedPoint_ = picked;
                isDraggingPoint_ = true;
            } else {
                selectedPoint_ = {-1, -1};
                isDraggingPoint_ = false;
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            isDraggingPoint_ = false;
        }

        // Handle dragging selected control point
        if (isDraggingPoint_ && selectedPoint_.first >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            auto vpRect = viewport_->rect();
            ImVec2 mousePos = ImGui::GetMousePos();
            double localX = mousePos.x - vpRect.x;
            double localY = mousePos.y - vpRect.y;

            auto [rayOrigin, rayDir] = viewport_->screenToWorldRay(localX, localY);
            Point3d currentPos = surface_->controlPoint(selectedPoint_.first, selectedPoint_.second);

            Vector3d newPos = currentPos;

            if (editAxis_ == 0) {
                double t = (currentPos.y() - rayOrigin.y()) / rayDir.y();
                if (std::abs(t) > 1e-6) {
                    Vector3d hit = rayOrigin + t * rayDir;
                    newPos.x() = hit.x();
                    newPos.z() = hit.z();
                }
            } else if (editAxis_ == 1) {
                double t = (currentPos.z() - rayOrigin.z()) / rayDir.z();
                if (std::abs(t) > 1e-6) {
                    Vector3d hit = rayOrigin + t * rayDir;
                    newPos.x() = hit.x();
                    newPos.y() = hit.y();
                }
            } else if (editAxis_ == 2) {
                double t = (currentPos.x() - rayOrigin.x()) / rayDir.x();
                if (std::abs(t) > 1e-6) {
                    Vector3d hit = rayOrigin + t * rayDir;
                    newPos.y() = hit.y();
                    newPos.z() = hit.z();
                }
            } else {
                Vector3d toPoint = currentPos - rayOrigin;
                double t = toPoint.dot(rayDir);
                newPos = rayOrigin + t * rayDir;
            }

            surface_->setControlPoint(selectedPoint_.first, selectedPoint_.second, newPos);
            meshDirty_ = true;
        }
    }
    viewport_->end();
}

void SurfaceEditor::renderScene() {
    if (!viewport_ || !surface_) return;

    GLRenderer& r = viewport_->renderer();

    // Rebuild mesh if dirty
    if (meshDirty_) {
        surfaceMesh_ = surface_->generateMesh(resolutionU_, resolutionV_);
        controlNetMesh_ = surface_->generateControlNet();
        meshDirty_ = false;
    }

    // Draw grid
    if (showGrid_) {
        r.drawGrid(5.0f, 10);
    }

    // Draw axes
    if (showAxes_) {
        r.drawAxes(2.5f);
    }

    // Draw surface
    if (showSurface_) {
        if (wireframeMode_) {
            r.drawWireframe(surfaceMesh_);
        } else {
            r.drawMesh(surfaceMesh_);
        }
    }

    // Draw control net
    if (showControlNet_) {
        r.drawWireframe(controlNetMesh_);
    }

    // Draw control points
    if (showControlPoints_ && surface_) {
        uint32_t normalColor = Color4f(1.0f, 0.65f, 0.0f, 1.0f).toABGR();
        uint32_t selectedColor = Color4f(1.0f, 1.0f, 0.0f, 1.0f).toABGR();

        for (int j = 0; j < surface_->numRows(); ++j) {
            for (int i = 0; i < surface_->numCols(); ++i) {
                Point3d p = surface_->controlPoint(j, i);
                bool isSelected = (j == selectedPoint_.first && i == selectedPoint_.second);
                uint32_t col = isSelected ? selectedColor : normalColor;
                float size = isSelected ? 10.0f : 7.0f;
                r.drawPointColored(p, size, col);
            }
        }
    }
}

std::pair<int, int> SurfaceEditor::pickControlPoint(double screenX, double screenY) {
    if (!surface_ || !viewport_) return {-1, -1};

    auto [rayOrigin, rayDir] = viewport_->screenToWorldRay(screenX, screenY);

    double closestDist = pickRadius_ * pickRadius_; // squared in screen space
    std::pair<int, int> best = {-1, -1};

    for (int j = 0; j < surface_->numRows(); ++j) {
        for (int i = 0; i < surface_->numCols(); ++i) {
            Point3d p = surface_->controlPoint(j, i);

            auto t = raySphereIntersect(rayOrigin, rayDir, p, 0.15);
            if (t && *t > 0) {
                // Check screen distance as well
                auto screenPos = viewport_->worldToScreen(p);
                if (screenPos) {
                    float dx = static_cast<float>(screenX) - screenPos->x;
                    float dy = static_cast<float>(screenY) - screenPos->y;
                    float screenDist2 = dx * dx + dy * dy;
                    if (screenDist2 < closestDist) {
                        closestDist = screenDist2;
                        best = {j, i};
                    }
                }
            }
        }
    }

    return best;
}

std::optional<double> SurfaceEditor::raySphereIntersect(
    const Vector3d& rayOrigin, const Vector3d& rayDir,
    const Vector3d& center, double radius) const {

    Vector3d oc = rayOrigin - center;
    double a = rayDir.dot(rayDir);
    double b = 2.0 * oc.dot(rayDir);
    double c = oc.dot(oc) - radius * radius;
    double disc = b * b - 4 * a * c;

    if (disc < 0) return std::nullopt;

    double sqrtDisc = std::sqrt(disc);
    double t1 = (-b - sqrtDisc) / (2 * a);
    double t2 = (-b + sqrtDisc) / (2 * a);

    if (t1 > 0) return t1;
    if (t2 > 0) return t2;
    return std::nullopt;
}

void SurfaceEditor::rebuildMesh() {
    meshDirty_ = true;
}

void SurfaceEditor::resetToDefault() {
    // Create a 4x4 bicubic Bezier surface (saddle shape)
    SurfacePointGrid3d grid;
    double s = 1.5;

    // Row 0 (v=0)
    grid.push_back({
        Point3d(-3*s, -3*s, 0),
        Point3d(-1*s, -3*s, 2),
        Point3d( 1*s, -3*s, 2),
        Point3d( 3*s, -3*s, 0)
    });
    // Row 1 (v=1/3)
    grid.push_back({
        Point3d(-3*s, -1*s, 2),
        Point3d(-1*s, -1*s, 4),
        Point3d( 1*s, -1*s, 4),
        Point3d( 3*s, -1*s, 2)
    });
    // Row 2 (v=2/3)
    grid.push_back({
        Point3d(-3*s,  1*s, 2),
        Point3d(-1*s,  1*s, 4),
        Point3d( 1*s,  1*s, 4),
        Point3d( 3*s,  1*s, 2)
    });
    // Row 3 (v=1)
    grid.push_back({
        Point3d(-3*s,  3*s, 0),
        Point3d(-1*s,  3*s, 2),
        Point3d( 1*s,  3*s, 2),
        Point3d( 3*s,  3*s, 0)
    });

    surface_ = std::make_unique<BezierSurface>(grid);
    meshDirty_ = true;
    selectedPoint_ = {-1, -1};
}

} // namespace cagd
