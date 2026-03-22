#pragma once

// Main application class for managing curve editors

#include "../base/curve_editor.h"
#include "../editors/bezier_editor.h"
#include <memory>
#include <vector>
#include <string>

namespace cagd {

// Main application class
class Application {
public:
    Application();
    ~Application() = default;

    // Initialize the application
    void initialize();

    // Render the main UI
    void render();

    // Handle mouse input
    void handleMouseButton(int button, int action, int mods);
    void handleMousePosition(double xpos, double ypos);

    // Set screen dimensions
    void setScreenSize(int width, int height);

private:
    // Render curve selector (top left)
    void renderCurveSelector();

    // Render canvas area (right side)
    void renderCanvas();

    // Available editors
    std::vector<std::unique_ptr<CurveEditor>> editors_;

    // Current editor index
    int currentEditorIndex_;

    // Screen dimensions
    int screenWidth_;
    int screenHeight_;

    // Canvas area
    int canvasX_, canvasY_;
    int canvasWidth_, canvasHeight_;
};

} // namespace cagd
