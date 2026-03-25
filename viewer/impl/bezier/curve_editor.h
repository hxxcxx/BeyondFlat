#pragma once

// Bezier Curve Editor

#include "viewer/framework/base/curve_editor.h"
#include "src/bezier/bezier_curve.h"
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

    // Render canvas using ImGui draw list
    void renderCanvas(const ImVec2& canvasPos) override;

    // Handle mouse input
    void handleMouseButton(int button, int action, int mods, double xpos, double ypos) override;
    void handleMousePosition(double xpos, double ypos) override;

    // Get editor name
    std::string getName() const override { return "Bezier Curve"; }

    // Get description
    std::string getDescription() const override {
        return "Parametric curve defined by control points using Bernstein polynomials";
    }

    // Handle keyboard input
    void handleKey(int key, int action, int mods) override;

private:
    // Check if mouse is over a control point
    int findControlPoint(double mouseX, double mouseY);

    // Perform subdivision at current parameter
    void performSubdivision();

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

    // Subdivision parameter
    double subdivideParam_;
    bool showSubdividePoint_;

    // Subdivided curves (for display after subdivision)
    std::unique_ptr<BezierCurve2d> subdividedLeftCurve_;
    std::unique_ptr<BezierCurve2d> subdividedRightCurve_;
    bool showSubdividedCurves_;

    // Interaction state
    int selectedControlPoint_;
    bool isDragging_;

    // Control point size for interaction
    float controlPointSize_;
};

} // namespace cagd
