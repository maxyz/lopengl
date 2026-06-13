#ifndef SCENESTATE_H
#define SCENESTATE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "camera.h"

// Global state
struct SceneState {
        float width;
        float height;
        const char* title;
        glm::vec3 bgColor;
        Camera camera;
        double lastX, lastY;
        bool firstMouse, recalculateObjects;
        float shininess;
}; 

// Functions that modify or interact with the global state
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window, float deltaTime, Camera &camera);
void sortTransparent(bool recalculateObjects, auto transparentObjects, auto *sorted);

// Abstract class programs must inherit from
class AbstractSceneRenderer {
    public:
        virtual void init() = 0;
        virtual void renderScene(SceneState &state) = 0;
        virtual void showImGuiControls(SceneState &state) = 0;
        virtual void teardown() = 0;
        virtual ~AbstractSceneRenderer() = default;
};


#endif
