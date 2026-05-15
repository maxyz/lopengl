#ifndef KEY_COMMAND_H
#define KEY_COMMAND_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>

#include <iostream>

using uint = unsigned int;

enum InputMode {
    NORMAL,
    TOGGLE
};

class KeyCommand {

private:

    std::function<void()> activate;

protected:

    uint keyCode;

    bool inputMode;
    bool keyPreviouslyPressed = false;

public: 

    KeyCommand() = default;
    KeyCommand(uint code, std::function<void()> func, InputMode mode = NORMAL) : 
        keyCode(code),
        activate(func),
        inputMode(mode)
    {}

    virtual void detect(GLFWwindow *window) {

        switch (inputMode)
        {
        case NORMAL:

            if(glfwGetKey(window, keyCode) == GLFW_PRESS)
                activate();
            break;

        case TOGGLE:

            if(!keyPreviouslyPressed && glfwGetKey(window, keyCode) == GLFW_PRESS)
                activate();

            keyPreviouslyPressed = glfwGetKey(window, keyCode) == GLFW_PRESS;
            break;
        }
    }
};

class KeyWindowCommand : public KeyCommand {

private:

    std::function<void(GLFWwindow *)> activate;

public:

    KeyWindowCommand(uint code, std::function<void(GLFWwindow *)> func, InputMode mode = NORMAL) :
        activate(func) 
    {
        keyCode = code;
        inputMode = mode;
    }

    void detect(GLFWwindow *window) {
        
        switch (inputMode)
        {
        case NORMAL:

            if(glfwGetKey(window, keyCode) == GLFW_PRESS)
            
                activate(window);
            break;

        case TOGGLE:    

            if(!keyPreviouslyPressed && glfwGetKey(window, keyCode) == GLFW_PRESS)

                activate(window);

            keyPreviouslyPressed = glfwGetKey(window, keyCode) == GLFW_PRESS;
            break;
        }
    }
};

#endif