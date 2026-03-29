#pragma once

// B-Spline Curve Editor

#include "viewer/framework/base/curve_editor.h"
#include "src/bspline/bspline_curve.h"
#include <memory>

namespace cagd {

// B-Spline Curve Editor
class BSplineEditor : public CurveEditor {
public:
    BSplineEditor();
    ~BSplineEditor() = default;

    // EditorBase interface
    void initialize() override;
    void render() override;
    std::string getName() const override { return "B-Spline Curve"; }
    std::string getDescription() const override {
        return "Parametric curve defined by control points and knot vector using B-spline basis functions";
    }

    // CurveEditor 2D input interface
    void handleMouseButton(int button, int action, int mods, double xpos, double ypos) override;
    void handleMousePosition(double xpos, double ypos) override;
    void handleKey(int key, int action, int mods) override;

private:
    // Render 2D canvas window
    void renderCanvas(const ImVec2& canvasPos);

    // Render control panel
    void renderControlPanel();

    // Check if mouse is over a control point
    int findControlPoint(double mouseX, double mouseY);

    // Perform knot insertion at current parameter
    void performKnotInsertion();

    // Current B-spline curve
    std::unique_ptr<BSplineCurve2d> curve_;

    // GUI state
    bool showControlPoints_;
    bool showControlPolygon_;
    bool showKnotPoints_;
    bool showTangent_;
    bool showPointOnCurve_;

    // Tangent parameter
    double tangentParam_;

    // Point on curve parameter
    double pointOnCurveParam_;

    // Knot insertion parameter
    double knotInsertParam_;
    int knotMultiplicity_;
    bool showKnotInsertPoint_;

    // Interaction state
    int selectedControlPoint_;
    bool isDragging_;

    // Control point size for interaction
    float controlPointSize_;

    // Default degree
    int degree_;
};

} // namespace cagd
