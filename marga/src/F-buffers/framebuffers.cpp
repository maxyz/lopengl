#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <map>
#include <format>
#include <cmath>
#include "stb_image.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "lights.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "texture_vertices.cpp"

const int INITIAL_WIDTH = 1024;
const int INITIAL_HEIGHT = 768;

float width = (float) INITIAL_WIDTH, height = (float) INITIAL_HEIGHT;

struct Material {
    float specular[3];
    float shininess;
};

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
    width = (float) _width;
    height = (float) _height;

}

// Mouse input requires a number of global variables :-/
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f));
double lastX = 400, lastY = 300;
bool firstMouse = true;
bool recalculateObjects;


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

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    if (firstMouse) {
        firstMouse = false;
        return;
    }
    camera.ProcessMouseMovement(xoffset, yoffset);
    recalculateObjects = true;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Tab toggles capturing the mouse
    if ((key == GLFW_KEY_TAB) && (action == GLFW_RELEASE)) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &lastX, &lastY);
            firstMouse = true;
        }
    }
}

void processInput(GLFWwindow *window, float deltaTime, Camera &camera)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 2.5f * deltaTime;
    Camera_Movement direction = NONE;
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        direction = UP;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        direction = DOWN;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        direction = RIGHT;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        direction = LEFT;
    if(glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        direction = FORWARD;
    if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        direction = BACKWARD;

    if (direction != NONE) {
        camera.ProcessKeyboard(direction, deltaTime);
        recalculateObjects = true;
    }
}


void sortTransparent(bool recalculateObjects, auto transparentObjects, auto *sorted)
{
    if (recalculateObjects == false)
        return;
    sorted->clear();
    for (unsigned int i = 0; i < transparentObjects.size(); i++)
    {
        float distance = glm::length(camera.Position - transparentObjects[i]);
        sorted->insert({distance, transparentObjects[i]});
    }
    recalculateObjects = true;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "Framebuffers", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Capture the mouse and call the callback function
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, keyboard_callback);

    int version = gladLoadGL(glfwGetProcAddress);
    if (!version)
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Initial size and call back for resizing
    glViewport(0, 0, INITIAL_WIDTH, INITIAL_HEIGHT);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    // Configure depth testing
    glDepthFunc(GL_LESS);
    // Enable blending
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Enable Face Culling (drop backward faces)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); 

    // Read shaders
    //Shader ourShader("shaders/vertex.glsl", "shaders/multi-fragment.glsl");
    Shader ourShader("shaders/vertex.glsl", "shaders/textured-multi-lights.glsl");
    //Shader ourShader("shaders/vertex.glsl", "shaders/simple-blending.glsl");
    //Shader ourShader("shaders/vertex.glsl", "shaders/depth-buffer.glsl");
    Shader sourceShader("shaders/source-vertex.glsl", "shaders/source-frag.glsl");
    Shader quadShader("shaders/quad-vertex.glsl", "shaders/quad-edge-frag.glsl");



    // Create framebuffer and bind it
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // generate texture for the framebuffer
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, INITIAL_WIDTH, INITIAL_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Generate stencil and depth testing renderbuffer
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, INITIAL_WIDTH, INITIAL_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    quadShader.use();
    quadShader.setInt("screenTexture", 0);

    // Vertex definition for cubes
    // Create an Element Buffer Object
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    // Create a buffer object called VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    // Generate a Vertex array
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. then set our vertex attributes pointers (the vertices are in lighting_vertices.cpp)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Plane definition
    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.6f,  5.0f,  0.0f, 1.0f, 0.0f, 2.0f, 0.0f,
        -5.0f, -0.6f, -5.0f,  0.0f, 1.0f, 0.0f, 0.0f, 2.0f,
        -5.0f, -0.6f,  5.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

         5.0f, -0.6f,  5.0f,  0.0f, 1.0f, 0.0f, 2.0f, 0.0f,
         5.0f, -0.6f, -5.0f,  0.0f, 1.0f, 0.0f, 2.0f, 2.0f,	
        -5.0f, -0.6f, -5.0f,  0.0f, 1.0f, 0.0f, 0.0f, 2.0f
    };    
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // ..:: Create a light source object ::..
    // It has its own VAO.
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    Texture::flip_vertically();
    Texture marble = Texture("../media/marble.jpg", GL_RGB);
    Texture metal = Texture("../media/metal.png", GL_RGB);
    Texture wood = Texture("../media/container.jpg", GL_RGB);

    /*Texture grass = Texture("../media/grass.png", GL_RGBA);
    grass.set_wrap(GL_CLAMP_TO_EDGE);

    Texture windowPane = Texture("../media/blending_transparent_window.png", GL_RGBA);
    Texture windowSpecular = Texture("../media/transparent_window_specular.png", GL_RGBA);
    windowPane.set_wrap(GL_CLAMP_TO_EDGE);*/
    
    ourShader.use();
    ourShader.setInt("material.texture_diffuse1", 0);
    ourShader.setInt("material.texture_specular1", 1);

    std::vector<glm::vec3> transparentObjects;
    transparentObjects.push_back(glm::vec3(-1.5f,  0.0f, -0.48f));
    transparentObjects.push_back(glm::vec3( 1.5f,  0.0f,  0.51f));
    transparentObjects.push_back(glm::vec3( 0.0f,  0.0f,  0.7f));
    transparentObjects.push_back(glm::vec3(-0.3f,  0.0f, -2.3f));
    transparentObjects.push_back(glm::vec3( 0.5f,  0.0f, -0.6f));  

    std::multimap<float, glm::vec3> sorted;
    recalculateObjects = true;
    sortTransparent(recalculateObjects, transparentObjects, &sorted);

    // Starting material values
    Material material = { {0.5f, 0.5f, 0.5f}, 32 };

    // Starting lighting values (position, color, ambient, diffuse, specular, constant, linear, quadratic, cutoff)
    DirectionalLight directionalLight(glm::vec3(-0.2f, 1.0f, 0.3f));
    SpotLight spotLight;
    PositionalLight positionalLights[] = {
    	PositionalLight(glm::vec3( 1.7f,  1.2f,  2.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.5f),
	    PositionalLight(glm::vec3( 4.3f, -3.3f, -4.0f), glm::vec3( 0.0f,  0.0f,  1.0f), 0.2f, 0.5f),
    	PositionalLight(glm::vec3(-4.0f,  2.0f, -2.0f), glm::vec3( 1.0f,  0.0f,  0.0f), 0.2f, 0.5f),
    	PositionalLight(glm::vec3( 1.0f,  0.0f, -3.0f), glm::vec3( 0.0f,  1.0f,  0.0f), 0.2f, 0.5f)
    };  

    // Starting Background
    glm::vec3 backgroundColor = glm::vec3( 0.1f,  0.2f,  0.4f);

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.Fonts->AddFontFromFileTTF("../media/Roboto-Regular.ttf", 20);
    io.IniFilename = NULL; // Avoid creating imgui.ini

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Render loop
    while(!glfwWindowShouldClose(window))
    {
        // Calculate delta
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window, deltaTime, camera);

        // first pass: draw to the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
        glEnable(GL_DEPTH_TEST);

        // rendering commands here
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        ourShader.setVec3f("viewPos", glm::value_ptr(camera.Position));
        ourShader.setFloat("material.shininess", material.shininess);

        directionalLight.setShaderValues(ourShader, "dirLight");

        for (int i = 0; i < sizeof(positionalLights)/sizeof(PositionalLight); i++) {
            positionalLights[i].setShaderValues(ourShader, std::format("pointLights[{}]", i));
        }

        spotLight.position = camera.Position;
        spotLight.direction = camera.Front;
        spotLight.setShaderValues(ourShader, "spotLight");

        // Coordinate matrixes
        // ** View **
        glm::mat4 view = camera.GetViewMatrix();
        // ** Projection **
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Zoom), width/height, 0.1f, 100.0f);

        ourShader.setMatrix4fv("view", glm::value_ptr(view));
        ourShader.setMatrix4fv("projection", glm::value_ptr(projection));

        glm::mat4 model = glm::mat4(1.0f);

        // Draw 2 cubes
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wood.ID);
        model = glm::translate(model, glm::vec3(-1.5f, 0.0f, -1.5f));
        ourShader.setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f, 0.0f, -0.5f));
        ourShader.setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // Draw the lights on the screen
        sourceShader.use();
        sourceShader.setMatrix4fv("view", glm::value_ptr(view));
        sourceShader.setMatrix4fv("projection", glm::value_ptr(projection));

        for (int i = 0; i < sizeof(positionalLights)/sizeof(PositionalLight); i++) {
            PositionalLight light = positionalLights[i];
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, light.position);
            model = glm::scale(model, glm::vec3(0.2f));
            sourceShader.setMatrix4fv("model", glm::value_ptr(model));
            sourceShader.setVec3f("lightColor", glm::value_ptr(light.color));
            // draw the light cube object
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // The floor and the transparent objects are not culled
        glDisable(GL_CULL_FACE);

        ourShader.use();
        // floor
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, metal.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metal.ID);
        model = glm::mat4(1.0f);
        ourShader.setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Put the transparent objects into a map to get them sorted by distance
        /*
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, windowPane.ID);
    
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, windowSpecular.ID);
        sortTransparent(recalculateObjects, transparentObjects, &sorted);
        for(std::map<float,glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) 
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);				
            ourShader.setMatrix4fv("model", glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }*/
        glEnable(GL_CULL_FACE);

        // second pass: use the framebuffer as a texture and render that
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);
  
        quadShader.use();  
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);  

        // ImGui dock
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
        ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushItemWidth(150.0f);
        ImGui::LabelText("Pos","(%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::LabelText("Front","(%.2f, %.2f, %.2f)", camera.Front.x, camera.Front.y, camera.Front.z);
        // Only show the controls when the mouse is not captured by the camera
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            ImGui::ColorEdit3("Background", glm::value_ptr(backgroundColor));
            ImGui::SliderFloat("Shininess", &material.shininess, 0.0f, 256.0f);

            directionalLight.showImGuiControls("Directional Light");
            for (int i = 0; i < sizeof(positionalLights)/sizeof(PositionalLight); i++) {
                positionalLights[i].showImGuiControls(std::format("Positional Light {}", i));
            }
            spotLight.showImGuiControls("Spot Light");

            if (ImGui::Button("Desert"))
                SetLights("Desert", backgroundColor, directionalLight, positionalLights, spotLight);
            ImGui::SameLine();
            if (ImGui::Button("Factory"))
                SetLights("Factory", backgroundColor, directionalLight, positionalLights, spotLight);
            ImGui::SameLine();
            if (ImGui::Button("Horror"))
                SetLights("Horror", backgroundColor, directionalLight, positionalLights, spotLight);
            ImGui::SameLine();
            if (ImGui::Button("BioLab"))
                SetLights("BioLab", backgroundColor, directionalLight, positionalLights, spotLight);
        }
        ImGui::SeparatorText("");
        ImGui::Text("Press TAB to switch modes");
        ImGui::PopItemWidth();
        ImGui::End();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        // check and call events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Clean up buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &planeVBO);

    // Cleanup Imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean up
    glfwTerminate();
    return 0;
}

