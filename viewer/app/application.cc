#include "application.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cmath>

namespace cagd {

Application::Application()
    : currentEditorIndex_(0)
    , screenWidth_(1280)
    , screenHeight_(720)
    , canvasX_(350)
    , canvasY_(0)
    , canvasWidth_(930)
    , canvasHeight_(720) {

    // Create editors
    editors_.push_back(std::make_unique<BezierEditor>());

    // Initialize all editors
    for (auto& editor : editors_) {
        editor->setScreenSize(screenWidth_, screenHeight_);
        editor->initialize();
    }
}

void Application::initialize() {
    // Already initialized in constructor
}

void Application::render() {
    // Render curve selector (top left)
    renderCurveSelector();

    // Render current editor's control panel (bottom left)
    editors_[currentEditorIndex_]->renderControlPanel();

    // Render canvas area (right side)
    renderCanvas();
}

void Application::renderCurveSelector() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);

    ImGui::Begin("Curve Selector");

    ImGui::Text("Select Curve Type:");
    ImGui::Separator();

    for (size_t i = 0; i < editors_.size(); ++i) {
        if (ImGui::RadioButton(editors_[i]->getName().c_str(), currentEditorIndex_ == static_cast<int>(i))) {
            currentEditorIndex_ = static_cast<int>(i);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", editors_[i]->getDescription().c_str());
        }
    }

    ImGui::End();
}

void Application::renderCanvas() {
    // Save current ImGui context
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    // Create canvas window
    ImGui::SetNextWindowPos(ImVec2(canvasX_, canvasY_), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(canvasWidth_, canvasHeight_), ImGuiCond_FirstUseEver);

    ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    // Get canvas position and size
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    // Update canvas dimensions
    canvasX_ = static_cast<int>(canvasPos.x);
    canvasY_ = static_cast<int>(canvasPos.y);
    canvasWidth_ = static_cast<int>(canvasSize.x);
    canvasHeight_ = static_cast<int>(canvasSize.y);

    // Create invisible button for mouse interaction
    ImGui::InvisibleButton("Canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft);

    // Check if mouse is hovering over canvas
    bool isHovered = ImGui::IsItemHovered();

    // Render the current editor's curve
    if (isHovered || ImGui::IsItemActive()) {
        // Handle mouse input
        if (ImGui::IsMouseClicked(0)) {
            editors_[currentEditorIndex_]->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
        } else if (ImGui::IsMouseReleased(0)) {
            editors_[currentEditorIndex_]->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
        }

        if (ImGui::IsMouseDragging(0)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            editors_[currentEditorIndex_]->handleMousePosition(mousePos.x - canvasPos.x, mousePos.y - canvasPos.y);
        }
    }

    // Render the curve using OpenGL
    editors_[currentEditorIndex_]->render();

    ImGui::End();

    ImGui::PopStyleVar(2);
}

void Application::handleMouseButton(int button, int action, int mods) {
    editors_[currentEditorIndex_]->handleMouseButton(button, action, mods);
}

void Application::handleMousePosition(double xpos, double ypos) {
    // Adjust mouse position to canvas coordinates
    double canvasX = xpos - canvasX_;
    double canvasY = ypos - canvasY_;
    editors_[currentEditorIndex_]->handleMousePosition(canvasX, canvasY);
}

void Application::setScreenSize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;

    for (auto& editor : editors_) {
        editor->setScreenSize(width, height);
    }
}

} // namespace cagd
