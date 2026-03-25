#include "viewer/framework/application.h"
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
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_Always);

    ImGui::Begin("Curve Selector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

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
    ImGui::SetNextWindowPos(ImVec2(350, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(930, 720), ImGuiCond_Always);

    ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Get canvas position and size
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    // Update canvas dimensions
    canvasX_ = static_cast<int>(canvasPos.x);
    canvasY_ = static_cast<int>(canvasPos.y);
    canvasWidth_ = static_cast<int>(canvasSize.x);
    canvasHeight_ = static_cast<int>(canvasSize.y);

    // Update the current editor's canvas size
    editors_[currentEditorIndex_]->setScreenSize(canvasWidth_, canvasHeight_);

    // Create invisible button for mouse interaction
    ImGui::InvisibleButton("Canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    // Check if mouse is hovering over canvas
    bool isHovered = ImGui::IsItemHovered();

    // Only handle mouse input when hovering over canvas
    if (isHovered) {
        // Get mouse position relative to canvas
        ImVec2 mousePos = ImGui::GetMousePos();
        double canvasMouseX = mousePos.x - canvasPos.x;
        double canvasMouseY = mousePos.y - canvasPos.y;

        // Handle mouse input
        if (ImGui::IsMouseClicked(0)) {
            editors_[currentEditorIndex_]->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0, canvasMouseX, canvasMouseY);
        } else if (ImGui::IsMouseReleased(0)) {
            editors_[currentEditorIndex_]->handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0, canvasMouseX, canvasMouseY);
        }

        if (ImGui::IsMouseClicked(1)) {
            editors_[currentEditorIndex_]->handleMouseButton(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0, canvasMouseX, canvasMouseY);
        } else if (ImGui::IsMouseReleased(1)) {
            editors_[currentEditorIndex_]->handleMouseButton(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, 0, canvasMouseX, canvasMouseY);
        }

        if (ImGui::IsMouseDragging(0)) {
            editors_[currentEditorIndex_]->handleMousePosition(canvasMouseX, canvasMouseY);
        }
    }

    // Render the curve using ImGui draw list
    editors_[currentEditorIndex_]->renderCanvas(canvasPos);

    ImGui::End();

    ImGui::PopStyleVar(2);
}

void Application::handleMouseButton(int button, int action, int mods) {
    // Get current mouse position and convert to canvas coordinates
    double xpos, ypos;
    glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);

    // Adjust to canvas coordinates
    double canvasX = xpos - canvasX_;
    double canvasY = ypos - canvasY_;

    editors_[currentEditorIndex_]->handleMouseButton(button, action, mods, canvasX, canvasY);
}

void Application::handleMousePosition(double xpos, double ypos) {
    // Adjust mouse position to canvas coordinates
    double canvasX = xpos - canvasX_;
    double canvasY = ypos - canvasY_;
    editors_[currentEditorIndex_]->handleMousePosition(canvasX, canvasY);
}

void Application::handleKey(int key, int action, int mods) {
    editors_[currentEditorIndex_]->handleKey(key, action, mods);
}

void Application::setScreenSize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;

    for (auto& editor : editors_) {
        editor->setScreenSize(width, height);
    }
}

} // namespace cagd
