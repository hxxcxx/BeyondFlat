#pragma once

// Base class for curve editors

#include <imgui.h>
#include "src/cagd_types.h"
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
    }

protected:
    // Helper function to convert world to screen coordinates
    void worldToScreen(const Point2d& world, float& screenX, float& screenY) {
        screenX = static_cast<float>((world.x() - worldMinX_) / (worldMaxX_ - worldMinX_) * screenWidth_);
        screenY = static_cast<float>((world.y() - worldMinY_) / (worldMaxY_ - worldMinY_) * screenHeight_);
    }

    // Helper function to convert screen to world coordinates
    Point2d screenToWorld(double screenX, double screenY) {
        double worldX = worldMinX_ + (screenX / screenWidth_) * (worldMaxX_ - worldMinX_);
        double worldY = worldMinY_ + (screenY / screenHeight_) * (worldMaxY_ - worldMinY_);
        return Point2d(worldX, worldY);
    }

    // Screen dimensions
    int screenWidth_;
    int screenHeight_;

    // World coordinate bounds
    double worldMinX_, worldMaxX_;
    double worldMinY_, worldMaxY_;
};

} // namespace cagd
