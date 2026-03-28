#pragma once

// Unified editor interface for 2D curve and 3D surface editors.

#include <string>

namespace cagd {

// Abstract base class for all editors (2D curves, 3D surfaces, etc.)
class EditorBase {
public:
    virtual ~EditorBase() = default;

    // Initialize the editor (called after OpenGL context is ready)
    virtual void initialize() = 0;

    // Render the full editor UI (control panel + canvas/viewport)
    virtual void render() = 0;

    // Get editor display name
    virtual std::string getName() const = 0;

    // Get editor description (for tooltips)
    virtual std::string getDescription() const = 0;

    // Notify screen size change
    virtual void setScreenSize(int width, int height) {}
};

} // namespace cagd
