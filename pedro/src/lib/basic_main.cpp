#include "basic_main.h"

int main()
{
    AbstractEngine* engine = createEngine();

    glm::mat4 view;

    while(!engine->windowShouldClose())
    {
        engine->update();
        
        engine->renderScene();

        engine->swapBuffers();
        glfwPollEvents();    
    }
    engine->teardown();

    glfwTerminate();
    return 0;
}