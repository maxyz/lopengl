#ifndef IMGUI_DOCK_H
#define IMGUI_DOCK_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "scene_state.h"
#include "lights.h"

class ImguiDock {
    public:
        void init(GLFWwindow* window);
        void render(GLFWwindow* window, SceneState state, AbstractSceneRenderer* renderer);
        void teardown();
};

#endif
