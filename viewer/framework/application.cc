#include "viewer/framework/application.h"
#include "viewer/impl/bezier/curve_editor.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cmath>

namespace cagd {

Application::Application()
    : currentMode_(0)
    , screenWidth_(1280)
    , screenHeight_(720) {

    // Create 2D curve editors
    curveEditors_.push_back(std::make_unique<BezierEditor>());

    // Create 3D surface editor
    surfaceEditor_ = std::make_unique<SurfaceEditor>();
}

void Application::initialize() {
    // Initialize all 2D curve editors
    for (auto& editor : curveEditors_) {
        editor->setScreenSize(screenWidth_, screenHeight_);
        editor->initialize();
    }

    // Initialize 3D surface editor
    surfaceEditor_->initialize();
}

void Application::render() {
    renderEditorSelector();
    renderCurrentEditor();
}

void Application::renderEditorSelector() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(330, 100), ImGuiCond_FirstUseEver);

    ImGui::Begin("Editor Selector", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Select Editor:");
    ImGui::Separator();

    // 2D curve editors
    for (size_t i = 0; i < curveEditors_.size(); ++i) {
        int mode = static_cast<int>(i);
        if (ImGui::RadioButton(curveEditors_[i]->getName().c_str(), currentMode_ == mode)) {
            currentMode_ = mode;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", curveEditors_[i]->getDescription().c_str());
        }
    }

    // 3D surface editor
    int surfaceMode = -1;
    if (ImGui::RadioButton(surfaceEditor_->getName().c_str(), currentMode_ == surfaceMode)) {
        currentMode_ = surfaceMode;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", surfaceEditor_->getDescription().c_str());
    }

    ImGui::End();
}

void Application::renderCurrentEditor() {
    if (currentMode_ >= 0 && currentMode_ < static_cast<int>(curveEditors_.size())) {
        // 2D curve editor mode
        auto& editor = curveEditors_[currentMode_];
        editor->renderControlPanel();

        // Render 2D canvas (same as before)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

        ImGui::SetNextWindowPos(ImVec2(330, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();

        editor->setScreenSize(static_cast<int>(canvasSize.x), static_cast<int>(canvasSize.y));

        ImGui::InvisibleButton("Canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

        bool isHovered = ImGui::IsItemHovered();
        if (isHovered) {
            ImVec2 mousePos = ImGui::GetMousePos();
            double canvasMouseX = mousePos.x - canvasPos.x;
            double canvasMouseY = mousePos.y - canvasPos.y;

            if (ImGui::IsMouseClicked(0)) {
                editor->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0, canvasMouseX, canvasMouseY);
            } else if (ImGui::IsMouseReleased(0)) {
                editor->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0, canvasMouseX, canvasMouseY);
            }
            if (ImGui::IsMouseClicked(1)) {
                editor->handleMouseButton(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0, canvasMouseX, canvasMouseY);
            } else if (ImGui::IsMouseReleased(1)) {
                editor->handleMouseButton(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, 0, canvasMouseX, canvasMouseY);
            }
            if (ImGui::IsMouseDragging(0)) {
                editor->handleMousePosition(canvasMouseX, canvasMouseY);
            }
        }

        editor->renderCanvas(canvasPos);

        ImGui::End();
        ImGui::PopStyleVar(2);
    } else if (currentMode_ == -1) {
        // 3D surface editor mode
        surfaceEditor_->render();
    }
}

void Application::handleMouseButton(int button, int action, int mods) {
    if (currentMode_ >= 0 && currentMode_ < static_cast<int>(curveEditors_.size())) {
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        curveEditors_[currentMode_]->handleMouseButton(button, action, mods, xpos, ypos);
    }
    // 3D editor handles its own mouse input through ImGui
}

void Application::handleMousePosition(double xpos, double ypos) {
    if (currentMode_ >= 0 && currentMode_ < static_cast<int>(curveEditors_.size())) {
        curveEditors_[currentMode_]->handleMousePosition(xpos, ypos);
    }
}

void Application::handleKey(int key, int action, int mods) {
    if (currentMode_ >= 0 && currentMode_ < static_cast<int>(curveEditors_.size())) {
        curveEditors_[currentMode_]->handleKey(key, action, mods);
    }
}

void Application::setScreenSize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    for (auto& editor : curveEditors_) {
        editor->setScreenSize(width, height);
    }
}

} // namespace cagd
