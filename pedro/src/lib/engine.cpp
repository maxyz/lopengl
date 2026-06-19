#include "engine.h"

AbstractEngine::AbstractEngine()
{
    basicInit();
    sceneInit();
}

AbstractEngine::~AbstractEngine()
{
    teardown();
    for(auto ptr : basicCommands) delete ptr;
    for(auto ptr : sceneCommands) delete ptr;
}

void AbstractEngine::handleFramebufferSizeCallback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
    state.width = _width;
    state.height = _height;
}

void AbstractEngine::handleMouseCallback(GLFWwindow* window, double xpos, double ypos) 
{
    if (!state.mouseLocked) return;

    if (state.firstMouse)
    {
        state.lastX = xpos;
        state.lastY = ypos;
        state.firstMouse = false;
    }

    float xoffset = xpos - state.lastX;
    float yoffset = state.lastY - ypos; 
    state.lastX = xpos;
    state.lastY = ypos;

    state.cam.handleMouseMovement(xoffset, yoffset);
}

void AbstractEngine::handleScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    state.cam.handleMouseScroll(yoffset);
}

void AbstractEngine::basicInit() 
{
    // State
    state = 
    {
        .window = nullptr,
        .width = W_WITDH,
        .height = W_HEIGHT,
        .lastX = W_WITDH / 2.0,
        .lastY = W_HEIGHT / 2.0,
        .deltaTime = 0.0f,
        .lastFrame = 0.0f,
        
        .firstMouse = true,
        .mouseLocked = true,
        .shaderNeedsToBeChanged = false
    };

    initWindow();
    updateProjection();
    
    // Commands
    basicCommands = {
        new KeyCommand(GLFW_KEY_R, [this]() -> void { state.cam.resetFov(); }, TOGGLE),
        
        new KeyCommand(GLFW_KEY_UP, [this]() -> void { state.cam.speed += 0.1f; }, NORMAL),

        new KeyCommand(GLFW_KEY_DOWN, [this]() -> void { state.cam.speed -= 0.1f; }, NORMAL),

        new KeyCommand(GLFW_KEY_W, [this]() -> void { state.cam.moveFront(); }, NORMAL),
                    
        new KeyCommand(GLFW_KEY_S, [this]() -> void { state.cam.moveBack(); }, NORMAL),

        new KeyCommand(GLFW_KEY_A, [this]() -> void { state.cam.moveLeft(); }, NORMAL),

        new KeyCommand(GLFW_KEY_D, [this]() -> void { state.cam.moveRight(); }, NORMAL),

        new KeyCommand(GLFW_KEY_SPACE, [this]() -> void { state.cam.moveWorldUp(); }, NORMAL),

        new KeyCommand(GLFW_KEY_LEFT_SHIFT, [this]() -> void { state.cam.moveWorldDown(); }, NORMAL),

        new KeyWindowCommand(GLFW_KEY_TAB, 
            [this](GLFWwindow *window) -> void { 
                if (state.mouseLocked) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    state.mouseLocked = false;
                    state.firstMouse = true;
                } else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    state.mouseLocked = true;
                }
            }, TOGGLE),

        new KeyWindowCommand(GLFW_KEY_ESCAPE, [this](GLFWwindow *window) -> void { glfwSetWindowShouldClose(window, true); }, TOGGLE)
    };
}

void AbstractEngine::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    state.window = glfwCreateWindow(state.width, state.height, "CrazyWindowAction", NULL, NULL);

    if (state.window == NULL)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(state.window,this);
    glfwMakeContextCurrent(state.window);
    
    glfwSetFramebufferSizeCallback(state.window, framebufferSizeCallback);
    glfwSetCursorPosCallback(state.window, mouseCallback);
    glfwSetScrollCallback(state.window, scrollCallback);

    glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }
}

void AbstractEngine::updateProjection()
{
    state.projectionMatrix = glm::perspective(glm::radians(state.cam.fov), state.width / state.height, state.cam.near, state.cam.far);
}

void AbstractEngine::processInput()
{
    state.cam.deltaSpeed = state.cam.speed * state.deltaTime;
    for (auto command : basicCommands)
    {
        command->detect(state.window);
    }
    for (auto command : sceneCommands)
    {
        command->detect(state.window);
    }
}

void AbstractEngine::updateFrames()
{
    float currentFrame = (float)glfwGetTime();
    state.deltaTime = currentFrame - state.lastFrame;
    state.lastFrame = currentFrame;
}

void framebufferSizeCallback(GLFWwindow* window, int _width, int _height)
{
    AbstractEngine *engine = (AbstractEngine*)glfwGetWindowUserPointer(window);
    engine->handleFramebufferSizeCallback(window, _width, _height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) 
{
    AbstractEngine *engine = (AbstractEngine*)glfwGetWindowUserPointer(window);
    engine->handleMouseCallback(window,xpos,ypos);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    AbstractEngine *engine = (AbstractEngine*)glfwGetWindowUserPointer(window);
    engine->handleScrollCallback(window, xoffset, yoffset);
}
