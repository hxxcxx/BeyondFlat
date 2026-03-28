#include "viewer/framework/application.h"
#include "viewer/impl/bezier/curve_editor.h"
#include "viewer/impl/bezier/surface_editor.h"
#include <imgui.h>

namespace cagd {

Application::Application() {
    editors_.push_back(std::make_unique<BezierEditor>());
    editors_.push_back(std::make_unique<SurfaceEditor>());
}

void Application::initialize() {
    for (auto& editor : editors_) {
        editor->initialize();
    }
}

void Application::render() {
    renderEditorSelector();
    if (currentEditor_ >= 0 && currentEditor_ < static_cast<int>(editors_.size())) {
        editors_[currentEditor_]->render();
    }
}

void Application::renderEditorSelector() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(330, 100), ImGuiCond_FirstUseEver);

    ImGui::Begin("Editor Selector", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Select Editor:");
    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(editors_.size()); ++i) {
        if (ImGui::RadioButton(editors_[i]->getName().c_str(), currentEditor_ == i)) {
            currentEditor_ = i;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", editors_[i]->getDescription().c_str());
        }
    }

    ImGui::End();
}

void Application::handleMouseButton(int, int, int) {
    // Each editor handles its own input via ImGui
}

void Application::handleMousePosition(double, double) {
    // Each editor handles its own input via ImGui
}

void Application::handleKey(int, int, int) {
    // Each editor handles its own input via ImGui
}

void Application::setScreenSize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    for (auto& editor : editors_) {
        editor->setScreenSize(width, height);
    }
}

} // namespace cagd
