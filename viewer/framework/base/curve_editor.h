#pragma once

// Base class for 2D curve editors

#include "viewer/framework/base/editor_base.h"
#include <imgui.h>
#include "src/cagd_types.h"
#include <memory>

namespace cagd {

// Abstract base class for 2D curve editors
class CurveEditor : public EditorBase {
public:
    ~CurveEditor() override = default;

    // Handle mouse input
    virtual void handleMouseButton(int button, int action, int mods, double xpos, double ypos) = 0;
    virtual void handleMousePosition(double xpos, double ypos) = 0;

    // Handle keyboard input (optional, default implementation does nothing)
    virtual void handleKey(int key, int action, int mods) {}

    // Set screen dimensions
    void setScreenSize(int width, int height) override {
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
