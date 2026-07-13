#include "imgui_dock.h"

void ImguiDock::init(GLFWwindow* window) {

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
}

void ImguiDock::render(GLFWwindow* window, SceneState &state, AbstractSceneRenderer* renderer) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
    ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(150.0f);
    ImGui::LabelText("Pos","(%.2f, %.2f, %.2f)", state.camera.Position.x, state.camera.Position.y, state.camera.Position.z);
    ImGui::LabelText("Front","(%.2f, %.2f, %.2f)", state.camera.Front.x, state.camera.Front.y, state.camera.Front.z);
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    // Only show the controls when the mouse is not captured by the camera
    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
        ImGui::ColorEdit3("Background", glm::value_ptr(state.bgColor));
        ImGui::SliderFloat("Shininess", &state.shininess, 0.0f, 256.0f);

        // Additional ImGui controls
        renderer->showImGuiControls(state);
    }
    ImGui::SeparatorText("");
    ImGui::Text("Press TAB to switch modes");
    ImGui::PopItemWidth();
    ImGui::End();
    // Once it's setup, render it to screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiDock::teardown() {
    // Cleanup Imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

