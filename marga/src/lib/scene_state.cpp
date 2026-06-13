#include "scene_state.h"

extern SceneState state;

// Width and Height handling
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
    state.width = (float) _width;
    state.height = (float) _height;

}

// Handle mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Let ImGui handle the mouse
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;  // let ImGui handle it

    // Only move the camera when the cursor is captured
    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        return;

    float xoffset = xpos - state.lastX;
    float yoffset = state.lastY - ypos; // reversed since y-coordinates range from bottom to top
    state.lastX = xpos;
    state.lastY = ypos;

    if (state.firstMouse) {
        state.firstMouse = false;
        return;
    }
    state.camera.ProcessMouseMovement(xoffset, yoffset);
    state.recalculateObjects = true;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    state.camera.ProcessMouseScroll(yoffset);
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Tab toggles capturing the mouse
    if ((key == GLFW_KEY_TAB) && (action == GLFW_RELEASE)) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &state.lastX, &state.lastY);
            state.firstMouse = true;
        }
    }
}

void processInput(GLFWwindow *window, float deltaTime, Camera &camera)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    Camera_Movement direction = NONE;
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        direction = FORWARD;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        direction = BACKWARD;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        direction = RIGHT;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        direction = LEFT;
    if(glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        direction = UP;
    if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        direction = DOWN;

    if (direction != NONE) {
        camera.ProcessKeyboard(direction, deltaTime);
        state.recalculateObjects = true;
    }
}

// Sort an array of transparent objects according to their distance to the camera
void sortObjects(bool recalculate, std::vector<glm::vec3> objects, std::multimap<float, glm::vec3> *sorted)
{
    if (recalculate == false)
        return;
    sorted->clear();
    for (unsigned int i = 0; i < objects.size(); i++)
    {
        float distance = glm::length(state.camera.Position - objects[i]);
        sorted->insert({distance, objects[i]});
    }
    recalculate = true;
}

