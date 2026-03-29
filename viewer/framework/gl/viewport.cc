#include "viewer/framework/gl/viewport.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>

namespace cagd {

Viewport3D::Viewport3D()
    : vpX_(0), vpY_(0), vpWidth_(0), vpHeight_(0)
    , rendererInitialized_(false)
    , beginSucceeded_(false)
    , isHovered_(false)
    , isDraggingRotate_(false)
    , isDraggingPan_(false)
    , lastMousePos_(0, 0) {}

void Viewport3D::resize(int x, int y, int width, int height) {
    vpX_ = x;
    vpY_ = y;
    vpWidth_ = width;
    vpHeight_ = height;
    camera_.setAspect(static_cast<double>(width) / static_cast<double>(height));
}

bool Viewport3D::begin(const char* label, const ImVec2& size) {
    // Create an ImGui child window for the viewport
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (!ImGui::BeginChild(label, size, ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::PopStyleVar();
        return false;
    }

    // Get the actual position and size of the child window's content area
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 actualSize = ImGui::GetContentRegionAvail();

    if (actualSize.x < 1 || actualSize.y < 1) {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return false;
    }

    int newX = static_cast<int>(pos.x);
    int newY = static_cast<int>(pos.y);
    int newW = static_cast<int>(actualSize.x);
    int newH = static_cast<int>(actualSize.y);

    if (newX != vpX_ || newY != vpY_ || newW != vpWidth_ || newH != vpHeight_) {
        resize(newX, newY, newW, newH);
    }

    isHovered_ = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

    // Setup OpenGL viewport and scissor for the 3D area
    // Note: OpenGL viewport origin is bottom-left, ImGui is top-left
    GLFWwindow* win = glfwGetCurrentContext();
    int winW, winH;
    glfwGetFramebufferSize(win, &winW, &winH);
    int glY = winH - newY - newH;

    glViewport(newX, glY, newW, newH);
    glScissor(newX, glY, newW, newH);
    glEnable(GL_SCISSOR_TEST);

    // Initialize renderer on first frame
    if (!rendererInitialized_) {
        renderer_.initialize();
        rendererInitialized_ = true;
    }

    // Update camera and renderer
    renderer_.setCamera(camera_);
    renderer_.setViewport(newW, newH);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderer_.beginFrame();

    // Handle input
    handleInput();

    beginSucceeded_ = true;
    // Always return true if we got here - rendering should happen regardless of hover state
    // The hover state is still tracked for input handling
    return true;
}

void Viewport3D::end() {
    glDisable(GL_SCISSOR_TEST);

    // Restore full window viewport
    GLFWwindow* win = glfwGetCurrentContext();
    int winW, winH;
    glfwGetFramebufferSize(win, &winW, &winH);
    glViewport(0, 0, winW, winH);

    if (beginSucceeded_) {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        beginSucceeded_ = false;
    }
}

void Viewport3D::handleInput() {
    if (!isHovered_) {
        isDraggingRotate_ = false;
        isDraggingPan_ = false;
        return;
    }

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 delta(mousePos.x - lastMousePos_.x, mousePos.y - lastMousePos_.y);

    // Left drag: rotate
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
        camera_.onMouseDragRotate(delta.x, delta.y);
        isDraggingRotate_ = true;
    } else {
        isDraggingRotate_ = false;
    }

    // Middle drag: pan
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        camera_.onMouseDragPan(delta.x, delta.y);
        isDraggingPan_ = true;
    } else {
        isDraggingPan_ = false;
    }

    // Right drag: pan (alternative)
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right) && !ImGui::GetIO().WantCaptureMouse) {
        camera_.onMouseDragPan(delta.x, delta.y);
    }

    // Scroll: zoom
    if (ImGui::IsItemHovered()) {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0) {
            camera_.onMouseScroll(static_cast<double>(wheel));
        }
    }

    lastMousePos_ = mousePos;
}

Matrix4d Viewport3D::viewProj() const {
    return camera_.projectionMatrix() * camera_.viewMatrix();
}

std::pair<Vector3d, Vector3d> Viewport3D::screenToWorldRay(double screenX, double screenY) const {
    // Convert screen coords to NDC
    double ndcX = (2.0 * screenX / vpWidth_) - 1.0;
    double ndcY = 1.0 - (2.0 * screenY / vpHeight_); // flip Y

    Matrix4d invVP = renderer_.invViewProjMatrix();

    // Near point (z = -1 in NDC for OpenGL)
    Vector4d nearNDC(ndcX, ndcY, -1.0, 1.0);
    Vector4d nearWorld = invVP * nearNDC;
    nearWorld /= nearWorld.w();

    // Far point (z = 1 in NDC)
    Vector4d farNDC(ndcX, ndcY, 1.0, 1.0);
    Vector4d farWorld = invVP * farNDC;
    farWorld /= farWorld.w();

    Vector3d rayOrigin = nearWorld.head<3>();
    Vector3d rayDir = (farWorld.head<3>() - rayOrigin).normalized();

    return {rayOrigin, rayDir};
}

std::optional<ImVec2> Viewport3D::worldToScreen(const Vector3d& worldPos) const {
    Matrix4d mvp = camera_.projectionMatrix() * camera_.viewMatrix();
    Vector4d clip = mvp * Vector4d(worldPos(0), worldPos(1), worldPos(2), 1.0);

    if (clip.w() <= 0) return std::nullopt; // behind camera

    double ndcX = clip.x() / clip.w();
    double ndcY = clip.y() / clip.w();

    // Check if in NDC range
    if (ndcX < -1 || ndcX > 1 || ndcY < -1 || ndcY > 1) return std::nullopt;

    // Convert to viewport coordinates (ImGui-style, top-left origin)
    double screenX = (ndcX + 1.0) * 0.5 * vpWidth_;
    double screenY = (1.0 - ndcY) * 0.5 * vpHeight_;

    return ImVec2(static_cast<float>(screenX), static_cast<float>(screenY));
}

} // namespace cagd
