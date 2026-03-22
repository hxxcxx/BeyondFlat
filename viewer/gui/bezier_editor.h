#pragma once

// Bezier Curve Editor GUI using ImGui

#include "../../src/cagd_types.h"
#include "../../src/bezier/bezier_curve.h"
#include "../render/bezier_renderer.h"
#include <memory>

namespace cagd {

// Bezier Curve Editor
class BezierEditor {
public:
    BezierEditor();

    // Initialize the editor
    void initialize();

    // Render the GUI
    void render();

    // Handle mouse input
    void handleMouseButton(int button, int action, int mods);
    void handleMousePosition(double xpos, double ypos);

    // Get the current Bezier curve
    const BezierCurve2d& getCurve() const { return *curve_; }

    // Set screen dimensions
    void setScreenSize(int width, int height);

private:
    // Render control panel
    void renderControlPanel();

    // Render info panel
    void renderInfoPanel();

    // Render De Casteljau visualization
    void renderDeCasteljauVisualization();

    // Check if mouse is over a control point
    int findControlPoint(double mouseX, double mouseY);

    // Convert screen coordinates to world coordinates
    Point2d screenToWorld(double screenX, double screenY);

    // Current Bezier curve (use pointer to avoid default constructor)
    std::unique_ptr<BezierCurve2d> curve_;

    // Renderer
    std::unique_ptr<BezierRenderer> renderer_;

    // GUI state
    bool showControlPoints_;
    bool showControlPolygon_;
    bool showTangent_;
    bool showDeCasteljau_;

    // Tangent parameter
    double tangentParam_;

    // De Casteljau parameter
    double deCasteljauParam_;

    // Interaction state
    int selectedControlPoint_;
    bool isDragging_;

    // Screen dimensions
    int screenWidth_;
    int screenHeight_;

    // World coordinate bounds
    double worldMinX_, worldMaxX_;
    double worldMinY_, worldMaxY_;

    // Control point size for interaction
    float controlPointSize_;
};

} // namespace cagd
