#pragma once

// Base class for curve editors

#include "curve_renderer.h"
#include <imgui.h>
#include <memory>
#include <string>

namespace cagd {

// Abstract base class for curve editors
class CurveEditor {
public:
    virtual ~CurveEditor() = default;

    // Initialize the editor
    virtual void initialize() = 0;

    // Render the GUI and curve
    virtual void render() = 0;

    // Render control panel (left side)
    virtual void renderControlPanel() = 0;

    // Render canvas (right side) using ImGui draw list
    virtual void renderCanvas(const ImVec2& canvasPos) = 0;

    // Handle mouse input
    virtual void handleMouseButton(int button, int action, int mods, double xpos, double ypos) = 0;
    virtual void handleMousePosition(double xpos, double ypos) = 0;

    // Get editor name
    virtual std::string getName() const = 0;

    // Get description
    virtual std::string getDescription() const = 0;

    // Set screen dimensions
    void setScreenSize(int width, int height) {
        screenWidth_ = width;
        screenHeight_ = height;
        if (renderer_) {
            renderer_->setScreenSize(width, height);
        }
    }

    // Get the renderer
    CurveRenderer* getRenderer() { return renderer_.get(); }

protected:
    // Renderer for this curve type
    std::unique_ptr<CurveRenderer> renderer_;

    // Screen dimensions
    int screenWidth_;
    int screenHeight_;

    // World coordinate bounds
    double worldMinX_, worldMaxX_;
    double worldMinY_, worldMaxY_;
};

} // namespace cagd
