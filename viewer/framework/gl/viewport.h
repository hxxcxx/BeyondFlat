#pragma once

// 3D Viewport widget for embedding OpenGL rendering inside an ImGui window
// Handles viewport scissor, mouse-to-world unprojection, and camera interaction

#include "viewer/framework/gl/camera.h"
#include "viewer/framework/gl/renderer.h"
#include <imgui.h>

namespace cagd {

struct Viewport3D {
    // Initialize camera and renderer
    Viewport3D();

    // Set viewport size (call on resize)
    void resize(int x, int y, int width, int height);

    // Begin 3D rendering into the current ImGui window
    // Returns true if the viewport is hovered and should receive input
    bool begin(const char* label, const ImVec2& size);

    // End 3D rendering, restore ImGui state
    void end();

    // Process ImGui mouse input for camera control
    void handleInput();

    // Get current view-projection matrix
    Matrix4d viewProj() const;

    // Get renderer for drawing
    GLRenderer& renderer() { return renderer_; }
    const GLRenderer& renderer() const { return renderer_; }

    // Get camera for queries
    OrbitCamera& camera() { return camera_; }
    const OrbitCamera& camera() const { return camera_; }

    // Convert screen position (relative to viewport) to a world-space ray
    // Returns (rayOrigin, rayDirection)
    std::pair<Vector3d, Vector3d> screenToWorldRay(double screenX, double screenY) const;

    // Project a world-space point to screen coordinates relative to viewport
    // Returns nullopt if behind camera
    std::optional<ImVec2> worldToScreen(const Vector3d& worldPos) const;

    // Viewport rectangle (screen coordinates)
    struct Rect { int x, y, w, h; };
    Rect rect() const { return { vpX_, vpY_, vpWidth_, vpHeight_ }; }

private:
    OrbitCamera camera_;
    GLRenderer renderer_;
    bool rendererInitialized_;

    // Viewport position and size (screen pixels)
    int vpX_, vpY_, vpWidth_, vpHeight_;

    // Interaction state
    bool isHovered_;
    bool isDraggingRotate_;
    bool isDraggingPan_;
    ImVec2 lastMousePos_;
};

} // namespace cagd
