#pragma once

// Bezier Surface 3D Editor
// Interactive editor for tensor-product Bezier surfaces in 3D

#include "viewer/framework/base/editor_base.h"
#include "src/bezier/bezier_surface.h"
#include "viewer/framework/gl/viewport.h"
#include <memory>
#include <string>

namespace cagd {

class SurfaceEditor : public EditorBase {
public:
    SurfaceEditor();
    ~SurfaceEditor() = default;

    // EditorBase interface
    void initialize() override;
    void render() override;
    std::string getName() const override { return "Bezier Surface"; }
    std::string getDescription() const override {
        return "Tensor-product Bezier surface defined by a grid of 3D control points";
    }

private:
    // Render left control panel
    void renderControlPanel();

    // Render 3D viewport
    void renderViewport();

    // Render the surface and decorations using the GL renderer
    void renderScene();

    // Find closest control point to a screen position
    // Returns (row, col) or (-1, -1) if none found
    std::pair<int, int> pickControlPoint(double screenX, double screenY);

    // Ray-sphere intersection for picking
    std::optional<double> raySphereIntersect(
        const Vector3d& rayOrigin, const Vector3d& rayDir,
        const Vector3d& center, double radius) const;

    // The Bezier surface
    std::unique_ptr<BezierSurface> surface_;

    // 3D viewport
    std::unique_ptr<Viewport3D> viewport_;

    // Cached meshes
    Mesh surfaceMesh_;
    Mesh controlNetMesh_;
    bool meshDirty_;

    // Visualization options
    bool showSurface_;
    bool showControlNet_;
    bool showControlPoints_;
    bool showAxes_;
    bool showGrid_;
    bool wireframeMode_;

    // Surface resolution
    int resolutionU_;
    int resolutionV_;

    // Interaction state
    std::pair<int, int> selectedPoint_;  // (-1, -1) if none selected
    bool isDraggingPoint_;
    double pickRadius_; // screen-space radius for picking

    // Editing mode
    int editAxis_; // 0 = XZ plane, 1 = XY plane, 2 = YZ plane, 3 = free (along view ray)

    void rebuildMesh();
    void resetToDefault();
};

} // namespace cagd
