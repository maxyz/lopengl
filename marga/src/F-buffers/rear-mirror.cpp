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
#include "geometry.h"
#include "scene_state.h"
//#include "imgui_dock.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

/*struct Material {
    float specular[3];
    float shininess;
};*/

const int INITIAL_WIDTH = 1024;
const int INITIAL_HEIGHT = 768;

SceneState state = {
        .width = (float) INITIAL_WIDTH,
        .height = (float) INITIAL_HEIGHT,
        .camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f)),
        .lastX = 400,
        .lastY = 300,
        .firstMouse = true,
        .shininess = 32.0,
};

class SceneRenderer {
    public:
        Shader  *sceneShader;
        Shader  *sourceShader;
        Shader  *quadShader;
        Texture *cubeMaterial;
        Texture *floorMaterial;
        LightSet *lights;
        unsigned int cubeVAO, cubeVBO;
        unsigned int planeVAO, planeVBO;
        unsigned int quadVAO, quadVBO;
        unsigned int lightVAO;

    void init();
    void renderScene(SceneState state);
    void teardown();
};

void SceneRenderer::init()
{
    // Read shaders
    this->sceneShader = new Shader("shaders/vertex.glsl", "shaders/textured-multi-lights.glsl");
    this->sourceShader = new Shader("shaders/source-vertex.glsl", "shaders/source-frag.glsl");
    this->quadShader = new Shader("shaders/quad-vertex.glsl", "shaders/quad-edge-frag.glsl");

    this->quadShader->use();
    this->quadShader->setInt("screenTexture", 0);

    Texture::flip_vertically();
    this->floorMaterial = new Texture("../media/marble.jpg", GL_RGB);
    this->cubeMaterial = new Texture("../media/container.jpg", GL_RGB);

    /*Texture grass = Texture("../media/grass.png", GL_RGBA);
    grass.set_wrap(GL_CLAMP_TO_EDGE);

    Texture windowPane = Texture("../media/blending_transparent_window.png", GL_RGBA);
    Texture windowSpecular = Texture("../media/transparent_window_specular.png", GL_RGBA);
    windowPane.set_wrap(GL_CLAMP_TO_EDGE);*/
    
    this->sceneShader->use();
    this->sceneShader->setInt("material.texture_diffuse1", 0);
    this->sceneShader->setInt("material.texture_specular1", 1);

    // VAOs and VBOs
    getCubeBuffers(&this->cubeVAO, &this->cubeVBO);
    getPlaneBuffers(&this->planeVAO, &this->planeVBO);
    getQuadBuffers(&this->quadVAO, &this->quadVBO);
    getLightBuffers(&this->lightVAO, this->cubeVBO);

    // Starting lighting values (position, color, ambient, diffuse, specular, constant, linear, quadratic, cutoff)
    DirectionalLight directionalLight(glm::vec3(-0.2f, 1.0f, 0.3f));
    SpotLight spotLight;
    std::array<PositionalLight, 4> positionalLights = {{
    	PositionalLight(glm::vec3( 1.7f,  1.2f,  2.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.5f),
	    PositionalLight(glm::vec3( 4.3f, -3.3f, -4.0f), glm::vec3( 0.0f,  0.0f,  1.0f), 0.2f, 0.5f),
    	PositionalLight(glm::vec3(-4.0f,  2.0f, -2.0f), glm::vec3( 1.0f,  0.0f,  0.0f), 0.2f, 0.5f),
    	PositionalLight(glm::vec3( 1.0f,  0.0f, -3.0f), glm::vec3( 0.0f,  1.0f,  0.0f), 0.2f, 0.5f)
    }};
    this->lights = new LightSet(directionalLight, spotLight, 4, positionalLights);

    // Transparent objects - not in use
    /*std::vector<glm::vec3> transparentObjects;
    transparentObjects.push_back(glm::vec3(-1.5f,  0.0f, -0.48f));
    transparentObjects.push_back(glm::vec3( 1.5f,  0.0f,  0.51f));
    transparentObjects.push_back(glm::vec3( 0.0f,  0.0f,  0.7f));
    transparentObjects.push_back(glm::vec3(-0.3f,  0.0f, -2.3f));
    transparentObjects.push_back(glm::vec3( 0.5f,  0.0f, -0.6f));  

    std::multimap<float, glm::vec3> sorted;
    sortTransparent(true, transparentObjects, &sorted);*/

}

void SceneRenderer::teardown()
{
    // Clean up buffers
    glDeleteVertexArrays(1, &this->cubeVAO);
    glDeleteVertexArrays(1, &this->planeVAO);
    glDeleteVertexArrays(1, &this->lightVAO);
    glDeleteVertexArrays(1, &this->quadVAO);
    glDeleteBuffers(1, &this->cubeVBO);
    glDeleteBuffers(1, &this->planeVBO);
    glDeleteBuffers(1, &this->quadVBO);
}

void SceneRenderer::renderScene(SceneState state) 
{
        glEnable(GL_DEPTH_TEST);

        // rendering commands here
        glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->sceneShader->use();
        this->sceneShader->setVec3f("viewPos", glm::value_ptr(state.camera.Position));
        this->sceneShader->setFloat("material.shininess", state.shininess);

        lights = this->lights;
        lights->directionalLight.setShaderValues(*this->sceneShader, "dirLight");

        for (int i = 0; i < lights->positionalLightAmount; i++) {
            lights->positionalLights[i].setShaderValues(*this->sceneShader, std::format("pointLights[{}]", i));
        }

        lights->spotLight.position = state.camera.Position;
        lights->spotLight.direction = state.camera.Front;
        lights->spotLight.setShaderValues(*this->sceneShader, "spotLight");

        // Coordinate matrixes
        // ** View **
        glm::mat4 view = state.camera.GetViewMatrix();
        // ** Projection **
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 100.0f);

        this->sceneShader->setMatrix4fv("view", glm::value_ptr(view));
        this->sceneShader->setMatrix4fv("projection", glm::value_ptr(projection));

        glm::mat4 model = glm::mat4(1.0f);

        // Draw 2 cubes
        glBindVertexArray(this->cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->cubeMaterial->ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->cubeMaterial->ID);
        model = glm::translate(model, glm::vec3(-1.5f, 0.0f, -1.5f));
        this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f, 0.0f, -0.5f));
        this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw the lights on the screen
        this->sourceShader->use();
        this->sourceShader->setMatrix4fv("view", glm::value_ptr(view));
        this->sourceShader->setMatrix4fv("projection", glm::value_ptr(projection));

        for (int i = 0; i < lights->positionalLightAmount; i++) {
            PositionalLight light = lights->positionalLights[i];
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, light.position);
            model = glm::scale(model, glm::vec3(0.2f));
            this->sourceShader->setMatrix4fv("model", glm::value_ptr(model));
            this->sourceShader->setVec3f("lightColor", glm::value_ptr(light.color));
            // draw the light cube object
            glBindVertexArray(this->lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // The floor and the transparent objects are not culled
        glDisable(GL_CULL_FACE);

        this->sceneShader->use();
        // floor
        glBindVertexArray(this->planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->floorMaterial->ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->floorMaterial->ID);
        model = glm::mat4(1.0f);
        this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
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
            this->sceneShader.setMatrix4fv("model", glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }*/
        glEnable(GL_CULL_FACE);
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

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame


    SceneRenderer renderer;
    renderer.init();

    // Starting Background
    state.bgColor = glm::vec3( 0.1f,  0.2f,  0.4f);

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
        processInput(window, deltaTime, state.camera);

        // first pass: draw to the framebuffer with the inverted camera
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        state.camera.setRearView();
        renderer.renderScene(state);
        state.camera.unsetRearView();

        // second pass: draw the main scene with the restored camera
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer
        glClear(GL_COLOR_BUFFER_BIT);
        renderer.renderScene(state);

        // And now draw the background mirror
        renderer.quadShader->use();  
        glBindVertexArray(renderer.quadVAO);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.8f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        renderer.quadShader->setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);  


        // ImGui dock
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
        ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushItemWidth(150.0f);
        ImGui::LabelText("Pos","(%.2f, %.2f, %.2f)", state.camera.Position.x, state.camera.Position.y, state.camera.Position.z);
        ImGui::LabelText("Front","(%.2f, %.2f, %.2f)", state.camera.Front.x, state.camera.Front.y, state.camera.Front.z);
        // Only show the controls when the mouse is not captured by the camera
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            ImGui::ColorEdit3("Background", glm::value_ptr(state.bgColor));
            ImGui::SliderFloat("Shininess", &state.shininess, 0.0f, 256.0f);

            renderer.lights->directionalLight.showImGuiControls("Directional Light");
            for (int i = 0; i < renderer.lights->positionalLightAmount; i++) {
                renderer.lights->positionalLights[i].showImGuiControls(std::format("Positional Light {}", i));
            }
            renderer.lights->spotLight.showImGuiControls("Spot Light");

            if (ImGui::Button("Desert"))
                SetLights("Desert", state.bgColor, renderer.lights);
            ImGui::SameLine();
            if (ImGui::Button("Factory"))
                SetLights("Factory", state.bgColor, renderer.lights);
            ImGui::SameLine();
            if (ImGui::Button("Horror"))
                SetLights("Horror", state.bgColor, renderer.lights);
            ImGui::SameLine();
            if (ImGui::Button("BioLab"))
                SetLights("BioLab", state.bgColor, renderer.lights);
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


    // Cleanup Imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean up
    glfwTerminate();
    return 0;
}

