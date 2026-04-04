#pragma once

// NURBS Curve Editor

#include "viewer/framework/base/curve_editor.h"
#include "src/nurbs/nurbs_curve.h"
#include <memory>

namespace cagd {

// NURBS Curve Editor with weight control
class NURBSEditor : public CurveEditor {
public:
    NURBSEditor();
    ~NURBSEditor() = default;

    // EditorBase interface
    void initialize() override;
    void render() override;
    std::string getName() const override { return "NURBS Curve"; }
    std::string getDescription() const override {
        return "Non-Uniform Rational B-Spline curve with weighted control points. "
               "Can exactly represent conic sections (circles, ellipses, etc.)";
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

    // Create a circle as an example NURBS curve
    void createCircleExample();

    // Create an ellipse example
    void createEllipseExample();

    // Create an arc example
    void createArcExample();

    // Aspect-ratio-preserving coordinate transforms
    void updateAspectScale();
    void worldToScreenAR(const Point2d& world, float& screenX, float& screenY);
    Point2d screenToWorldAR(double screenX, double screenY);

    // Current NURBS curve
    std::unique_ptr<NURBSCurve2d> curve_;

    // GUI state
    bool showControlPoints_;
    bool showControlPolygon_;
    bool showKnotPoints_;
    bool showTangent_;
    bool showPointOnCurve_;
    bool showWeightedInfluence_;

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
