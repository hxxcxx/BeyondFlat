#pragma once

// Bezier Curve Editor

#include "../base/curve_editor.h"
#include "../../src/bezier/bezier_curve.h"
#include "../render/bezier_renderer.h"
#include <memory>

namespace cagd {

// Bezier Curve Editor
class BezierEditor : public CurveEditor {
public:
    BezierEditor();
    ~BezierEditor() = default;

    // Initialize the editor
    void initialize() override;

    // Render the GUI and curve
    void render() override;

    // Render control panel (left side)
    void renderControlPanel() override;

    // Handle mouse input
    void handleMouseButton(int button, int action, int mods) override;
    void handleMousePosition(double xpos, double ypos) override;

    // Get editor name
    std::string getName() const override { return "Bezier Curve"; }

    // Get description
    std::string getDescription() const override {
        return "Parametric curve defined by control points using Bernstein polynomials";
    }

private:
    // Render info panel
    void renderInfoPanel();

    // Check if mouse is over a control point
    int findControlPoint(double mouseX, double mouseY);

    // Current Bezier curve
    std::unique_ptr<BezierCurve2d> curve_;

    // GUI state
    bool showControlPoints_;
    bool showControlPolygon_;
    bool showTangent_;
    bool showPointOnCurve_;

    // Tangent parameter
    double tangentParam_;

    // Point on curve parameter
    double pointOnCurveParam_;

    // Interaction state
    int selectedControlPoint_;
    bool isDragging_;

    // Control point size for interaction
    float controlPointSize_;
};

} // namespace cagd
