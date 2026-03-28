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

    // EditorBase interface
    void initialize() override;
    void render() override;
    std::string getName() const override { return "Bezier Curve"; }
    std::string getDescription() const override {
        return "Parametric curve defined by control points using Bernstein polynomials";
    }

    // CurveEditor 2D input interface
    void handleMouseButton(int button, int action, int mods, double xpos, double ypos) override;
    void handleMousePosition(double xpos, double ypos) override;
    void handleKey(int key, int action, int mods) override;

private:
    // Render 2D canvas window
    void renderCanvas();

    // Render control panel
    void renderControlPanel();

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

    // Degree reduction warning
    bool showReducedDegreeWarning_ = false;
    double reducedDegreeError_ = 0.0;
    std::unique_ptr<BezierCurve2d> previewReducedCurve_;

    // Control point size for interaction
    float controlPointSize_;
};

} // namespace cagd
