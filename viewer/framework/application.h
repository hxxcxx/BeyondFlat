#pragma once

// Main application class for managing curve/surface editors

#include "viewer/framework/base/curve_editor.h"
#include "viewer/impl/bezier/surface_editor.h"
#include <memory>
#include <vector>
#include <string>

namespace cagd {

// Main application class
class Application {
public:
    Application();
    ~Application() = default;

    // Initialize the application (call after OpenGL context is created)
    void initialize();

    // Render the main UI
    void render();

    // Handle mouse input
    void handleMouseButton(int button, int action, int mods);
    void handleMousePosition(double xpos, double ypos);

    // Handle keyboard input
    void handleKey(int key, int action, int mods);

    // Set screen dimensions
    void setScreenSize(int width, int height);

private:
    // Render editor selector (top left)
    void renderEditorSelector();

    // Render the current editor
    void renderCurrentEditor();

    // 2D curve editors
    std::vector<std::unique_ptr<CurveEditor>> curveEditors_;

    // 3D surface editor
    std::unique_ptr<SurfaceEditor> surfaceEditor_;

    // Current mode: 0..n-1 = curve editors, -1 = surface editor
    int currentMode_;

    // Screen dimensions
    int screenWidth_;
    int screenHeight_;
};

} // namespace cagd
