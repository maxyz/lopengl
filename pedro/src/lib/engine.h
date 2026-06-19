#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "key_command.h"
#include "camera.h"

#define W_WITDH 1280.0f
#define W_HEIGHT 720.0f

void debugPrint(std::string print);

struct sceneState
{
    GLFWwindow *window;
    float width;
    float height;
    float lastX;
    float lastY;
    float deltaTime;
    float lastFrame;
    Camera cam;
    glm::mat4 projectionMatrix;

    bool firstMouse;
    bool mouseLocked;
};

class AbstractEngine 
{
public:

    sceneState state;
    std::array<KeyCommand*, 11> basicCommands;
    std::vector<KeyCommand*> sceneCommands;

    // Constructors
    AbstractEngine() = default;
    ~AbstractEngine()
    {
    for(auto ptr : basicCommands) delete ptr;
    for(auto ptr : sceneCommands) delete ptr;
    }

    // Factory Method
    template <typename T, typename... Args>
    static T* create(Args&&... args) {
        auto instance = new T(std::forward<Args>(args)...);

        instance->basicInit();
        instance->sceneInit();

        return instance;
    }

    // Pure Virtual methods
    virtual void sceneInit() = 0;
    virtual void update() = 0;
    virtual void renderScene() = 0;
    virtual void teardown() = 0;

    // Methods
    inline bool windowShouldClose()
    {
        return glfwWindowShouldClose(state.window);
    }

    inline void swapBuffers()
    {
        glfwSwapBuffers(state.window);
    }

    void handleFramebufferSizeCallback(GLFWwindow*, int, int);
    void handleMouseCallback(GLFWwindow*, double, double);
    void handleScrollCallback(GLFWwindow*, double, double);

protected:

    void basicInit();
    void initWindow();
    void updateProjection();
    void processInput();
    void updateFrames();
};

void framebufferSizeCallback(GLFWwindow*, int, int);
void mouseCallback(GLFWwindow*, double, double);
void scrollCallback(GLFWwindow*, double, double);

#endif