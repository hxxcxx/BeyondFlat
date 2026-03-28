#pragma once

// Main application class for managing editors

#include "viewer/framework/base/editor_base.h"
#include <memory>
#include <vector>

namespace cagd {

class Application {
public:
    Application();
    ~Application() = default;

    void initialize();
    void render();

    // Input callbacks (for editors that need raw input)
    void handleMouseButton(int button, int action, int mods);
    void handleMousePosition(double xpos, double ypos);
    void handleKey(int key, int action, int mods);
    void setScreenSize(int width, int height);

private:
    void renderEditorSelector();

    std::vector<std::unique_ptr<EditorBase>> editors_;
    int currentEditor_ = 0;
    int screenWidth_ = 1280;
    int screenHeight_ = 720;
};

} // namespace cagd
