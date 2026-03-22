#pragma once

// Base class for curve renderers

#include "../../src/cagd_types.h"
#include <string>

namespace cagd {

// Abstract base class for curve rendering
class CurveRenderer {
public:
    virtual ~CurveRenderer() = default;

    // Render the curve
    virtual void renderCurve() = 0;

    // Render control points
    virtual void renderControlPoints() = 0;

    // Render control polygon
    virtual void renderControlPolygon() = 0;

    // Render additional visualizations (tangent, normals, etc.)
    virtual void renderVisualizations() = 0;

    // Get curve name
    virtual std::string getName() const = 0;

    // Public interface for coordinate conversion
    void convertWorldToScreen(const Point2d& world, float& screenX, float& screenY) {
        worldToScreen(world, screenX, screenY);
    }

    Point2d convertScreenToWorld(double screenX, double screenY) {
        return screenToWorld(screenX, screenY);
    }

    // Set screen dimensions
    void setScreenSize(int width, int height) {
        screenWidth_ = width;
        screenHeight_ = height;
    }

    // Set world coordinate bounds
    void setWorldBounds(double minX, double maxX, double minY, double maxY) {
        worldMinX_ = minX;
        worldMaxX_ = maxX;
        worldMinY_ = minY;
        worldMaxY_ = maxY;
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
