#include <cmath>
#include <expected>
#include <functional>
#include <glm/geometric.hpp>
#include <iostream>
#include <print>
#include <vector>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "common/assets.hpp"
#include "common/geometry.hpp"
#include "common/camera.hpp"
#include "common/light.hpp"
#include "common/materials.hpp"
#include "common/shader.hpp"

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 1024;
const GLuint HEIGHT = 768;

struct view_t {
  float width = WIDTH;
  float height = HEIGHT;
};

struct attenuation_t {
  float constant;
  float linear;
  float quadratic;
};

struct preset_t {
  std::string name;
  glm::vec4 clear_color;
  light_directional_t dir_light;
  light_spot_t spot_light;
  std::array<glm::vec3, 4> pos_lights_pos;
  std::array<glm::vec3, 4> pos_lights_color;
  attenuation_t pos_lights_attenuation;
  std::array<glm::vec3, 2> spot_lights_pos;
  std::array<glm::vec3, 2> spot_lights_dir;
};

const std::array<preset_t, 4> presets = {
    {
        {
            .name = "desert",
            .clear_color = glm::vec4(.75f, .52f, .3f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.3f, 0.24f, .14f),
                    .diffuse = glm::vec3(.7f, .42f, .26f),
                    .specular = glm::vec3(.5f, .5f, .5f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(.5f, .5f, .5f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .09f,
                    .quadratic = .032f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(1.f, .6f, 0.f),
                glm::vec3(1.f, 0.f, 0.f),
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(.2f, .2f, 1.f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
        {
            .name = "factory",
            .clear_color = glm::vec4(.1f, .1f, .1f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.05f, 0.05f, .1f),
                    .diffuse = glm::vec3(.2f, .2f, .7f),
                    .specular = glm::vec3(.7f, .7f, .7f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(1.f, 1.f, 1.f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .09f,
                    .quadratic = .032f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(.2f, .2f, .6f),
                glm::vec3(.3f, .3f, .7f),
                glm::vec3(.0f, .0f, .3f),
                glm::vec3(.4f, .4f, .4f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
        {
            .name = "horror",
            .clear_color = glm::vec4(.0f, .0f, .0f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.0f, 0.0f, .0f),
                    .diffuse = glm::vec3(.05f, .05f, .05f),
                    .specular = glm::vec3(.2f, .2f, .2f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(1.f, 1.f, 1.f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .09f,
                    .quadratic = .032f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(.1f, .1f, .1f),
                glm::vec3(.1f, .1f, .1f),
                glm::vec3(.1f, .1f, .1f),
                glm::vec3(.3f, .1f, .1f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
        {
            .name = "biochemical",
            .clear_color = glm::vec4(.9f, .9f, .9f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.5f, 0.5f, .5f),
                    .diffuse = glm::vec3(1.f, 1.f, 1.f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(.0f, 1.f, .0f),
                    .specular = glm::vec3(.0f, 1.f, .0f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .07f,
                    .quadratic = .017f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(.4f, .7f, .1f),
                glm::vec3(.4f, .7f, .1f),
                glm::vec3(.4f, .7f, .1f),
                glm::vec3(.4f, .7f, .1f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
    }};

struct state_t {
  view_t viewport = {
      .width = WIDTH,
      .height = HEIGHT,
  };
  Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));
  size_t preset_index = 0;
  bool mouse_new_focus = true;
};
// Global state
state_t state;

enum event_t {
  NONE = 0,
  increase_fov = 1 << 0,
  decrease_fov = 1 << 1,
  camera_up = 1 << 2,
  camera_down = 1 << 3,
  camera_left = 1 << 4,
  camera_right = 1 << 5,
  camera_for = 1 << 6,
  camera_back = 1 << 7,
  camera_yaw_left = 1 << 8,
  camera_yaw_right = 1 << 9,
  light_up = 1 << 10,
  light_down = 1 << 11,
  light_left = 1 << 12,
  light_right = 1 << 13,
  light_for = 1 << 14,
  light_back = 1 << 15,
};

using cb_t = std::function<void(uint64_t event, float delta)>;
using cbs_t = std::vector<cb_t>;
using cleanup_t = std::function<void()>;

struct hooks_t {
  cbs_t callbacks = {};
  cleanup_t cleanup = []() {};
};

std::expected<GLFWwindow *, std::string> init_window();
std::expected<hooks_t, std::string> init_shaders(GLFWwindow *);
std::expected<void, std::string> init_textures();

void event_loop(GLFWwindow *window, cbs_t cbs);

void cleanup(cleanup_t);

int main() {
  auto window = init_window();
  if (!window) {
    std::cerr << window.error() << std::endl;
    return -1;
  }

  auto res = init_shaders(*window);
  if (!res) {
    std::cerr << res.error() << std::endl;
    return -1;
  }
  event_loop(*window, res->callbacks);

  cleanup(res->cleanup);
  return 0;
}

void cleanup(cleanup_t cleanup_) {
  cleanup_();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
}

std::expected<GLFWwindow *, std::string> init_glfw_window(view_t viewport) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(viewport.width, viewport.height, TITLE, NULL, NULL);
  if (window == NULL) {
    glfwTerminate();
    return std::unexpected("failed to create GLFW window");
  }
  glfwMakeContextCurrent(window);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    glfwTerminate();
    return std::unexpected("failed to init glad on top of glfw");
  }
  glViewport(0, 0, viewport.width, viewport.height);
  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  return window;
}

std::expected<void, std::string> init_imgui(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  auto font_path = get_asset_path("fonts/NotoSans-Regular.ttf");
  if (!font_path) {
    glfwTerminate();
    return std::unexpected("failed to obtain default font");
  }
  io.Fonts->AddFontFromFileTTF(font_path->c_str(), 20);
  io.IniFilename = NULL;

  ImGui::StyleColorsClassic();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();

  return {};
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void window_focus_callback(GLFWwindow *window, int focused);

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void init_window_callbacks(GLFWwindow *window) {
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetWindowFocusCallback(window, window_focus_callback);

  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
}

std::expected<GLFWwindow *, std::string> init_window() {
  return init_glfw_window(state.viewport)
      .and_then([](GLFWwindow *w) -> std::expected<GLFWwindow *, std::string> {
        init_window_callbacks(w);
        return w;
      })
      .and_then([](GLFWwindow *w) {
        return init_imgui(w).transform([w] { return w; });
      });
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  state.viewport.width = width;
  state.viewport.height = height;
  glViewport(0, 0, width, height);
}

void window_focus_callback(GLFWwindow *window, int focused) {
  if (!focused) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    state.mouse_new_focus = true;
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) {
    return;
  }

  if ((key == GLFW_KEY_GRAVE_ACCENT) &&
      (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    auto input_mode = glfwGetInputMode(window, GLFW_CURSOR);
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      state.mouse_new_focus = true;
    }
  }

  if ((key == GLFW_KEY_P) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (mods & GLFW_MOD_SHIFT) {
      state.preset_index =
          (presets.size() + state.preset_index - 1) % presets.size();
    } else {
      state.preset_index = (state.preset_index + 1) % presets.size();
    }
  }
}

const float MOUSE_SENSITIVITY = .1f;

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
  static float x = state.viewport.width / 2;
  static float y = state.viewport.height / 2;

  ImGuiIO &io = ImGui::GetIO();
  auto input_mode = glfwGetInputMode(window, GLFW_CURSOR);
  if ((input_mode == GLFW_CURSOR_NORMAL) ||
      ((input_mode == GLFW_CURSOR_DISABLED) && io.WantCaptureMouse)) {
    return;
  }

  if (state.mouse_new_focus) {
    x = x_pos;
    y = y_pos;
    state.mouse_new_focus = false;
  }

  float x_offset = x_pos - x;
  float y_offset = y - y_pos;
  x = x_pos;
  y = y_pos;
  state.camera.process_rotation(x_offset, y_offset);
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse) {
    return;
  }

  state.camera.update_fov(static_cast<float>(y_offset));
}



const glm::vec3 cube_positions[] = {
    glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
    glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};


struct buffers_t {
  id_t cube_vao;
  id_t light_vao;
  id_t pyramid_vao;

  cleanup_t cleanup = []() {};
};

buffers_t buffers();
void process_events(uint64_t e, float delta);

light_positional_t get_pos_light(size_t i) {
  const preset_t &preset = presets[state.preset_index];
  return {
      .position = preset.pos_lights_pos[i],
      .ambient = preset.pos_lights_color[i] * .1f,
      .diffuse = preset.pos_lights_color[i],
      .specular = preset.pos_lights_color[i],
      .constant = preset.pos_lights_attenuation.constant,
      .linear = preset.pos_lights_attenuation.linear,
      .quadratic = preset.pos_lights_attenuation.quadratic,
  };
}

light_spot_t get_spot_light(size_t i) {
  const preset_t &preset = presets[state.preset_index];
  light_spot_t spot_light = preset.spot_light;
  spot_light.position = preset.spot_lights_pos[i];
  spot_light.direction = preset.spot_lights_dir[i];
  return spot_light;
}

std::expected<hooks_t, std::string> init_shaders(GLFWwindow *window) {
  struct programs_t {
    id_t view;
    id_t light;
  };
  struct vaos_t {
    id_t cube;
    id_t pyramid;
    id_t light;
  };
  struct textures_t {
    id_t diffuse;
    id_t specular;
  };
  auto shader =
      Shader::build("shaders/17_multiple.vert", "shaders/17_multiple.frag");
  if (!shader) {
    return std::unexpected(shader.error());
  }
  auto light_shader =
      Shader::build("shaders/17_light.vert", "shaders/17_light.frag");
  if (!light_shader) {
    return std::unexpected(light_shader.error());
  }
  programs_t ps{.view = shader->ID, .light = light_shader->ID};

  auto load_texture_res = load_texture("textures/container2.png");
  if (!load_texture_res) {
    return std::unexpected(load_texture_res.error());
  }
  auto load_texture_specular_res =
      load_texture("textures/container2_specular.png");
  if (!load_texture_specular_res) {
    return std::unexpected(load_texture_specular_res.error());
  }
  textures_t ts = {
      .diffuse = *load_texture_res,
      .specular = *load_texture_specular_res,
  };

  auto vaos = buffers();
  vaos_t vs = {.cube = vaos.cube_vao,
               .pyramid = vaos.pyramid_vao,
               .light = vaos.light_vao};

  auto f = [ps, vs, ts](uint64_t e, float delta) {
    auto now = glfwGetTime();
    process_events(e, delta);

    glUseProgram(ps.view);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ts.diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ts.specular);
    specular_map_t specular_map = {
        .diffuse = 0,
        .specular = 1,
        .shininess = 64.f,
    };

    glm::mat4 model;

    glm::mat4 view = state.camera.get_view_matrix();

    glm::mat4 projection =
        glm::perspective(glm::radians(state.camera.fov),
                         static_cast<float>(state.viewport.width) /
                             static_cast<float>(state.viewport.height),
                         .1f, 100.f);

    glUseProgram(ps.view);
    set_mat4(ps.view, "view", view);
    set_mat4(ps.view, "projection", projection);
    set_vec3(ps.view, "view_pos", state.camera.position);
    set_specular_map(ps.view, "material", specular_map);

    glUseProgram(ps.light);
    set_mat4(ps.light, "view", view);
    set_mat4(ps.light, "projection", projection);
    glBindVertexArray(vs.light);

    const preset_t &preset = presets[state.preset_index];
    const light_directional_t &dir_light = preset.dir_light;
    glUseProgram(ps.view);
    set_directional_light(ps.view, "dir_light", dir_light);

    // Set and draw positional lights
    for (unsigned int i = 0; i < preset.pos_lights_pos.size(); ++i) {
      light_positional_t pos_light = get_pos_light(i);
      glUseProgram(ps.view);
      set_positional_light(ps.view, std::format("pos_lights[{}]", i),
                           pos_light);
      glUseProgram(ps.light);
      model = glm::mat4(1.f);
      model = glm::translate(model, pos_light.position);
      model = glm::scale(model, glm::vec3(.2f));

      set_mat4(ps.light, "model", model);
      set_positional_light(ps.light, std::format("pos_lights[{}]", i),
                           pos_light);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glUseProgram(ps.light);
    glBindVertexArray(vs.pyramid);

    // Set and draw spot lights
    for (unsigned int i = 0; i < preset.spot_lights_pos.size(); ++i) {
      light_spot_t spot_light = get_spot_light(i);
      glUseProgram(ps.view);
      set_spot_light(ps.view, std::format("spot_lights[{}]", i), spot_light);

      glUseProgram(ps.light);
      model = glm::mat4(1.f);
      model = glm::translate(model, spot_light.position);
      glm::mat4 look_at_rotation =
          glm::lookAt(glm::vec3(0.f), spot_light.direction, glm::vec3(0, 1, 0));
      model = model * glm::inverse(look_at_rotation);
      model = glm::scale(model, glm::vec3(.2f));

      set_mat4(ps.light, "model", model);
      set_spot_light(ps.light, "light", spot_light);
      glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    }

    glUseProgram(ps.view);
    glBindVertexArray(vs.cube);
    float angle;
    for (unsigned int i = 0; i < 10; ++i) {
      angle = 20.f * i;
      angle = glfwGetTime() * (i % 3) * 25.f;
      model = glm::translate(glm::mat4(1.f), cube_positions[i]);
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

      set_mat4(ps.view, "model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  };

  auto imgui = [window](uint64_t e, float delta) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO &io = ImGui::GetIO();
    enum mode_t {
      mode_CAM,
      mode_GUI,
    };
    mode_t mode =
        (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            ? mode_CAM
            : mode_GUI;

    if (mode == mode_CAM) {
      io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    } else if (mode == mode_GUI) {
      io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(150.0f);

    ImGui::LabelText("Pos", "(%.2f, %.2f, %.2f)", state.camera.position.x,
                     state.camera.position.y, state.camera.position.z);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    ImGui::LabelText("Mouse", "(%.2f, %.2f)", x, y);
    if (mode == mode_GUI) {
    }
    ImGui::PopItemWidth();

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  };

  shader->use();
  shader->set_int("material.diffuse", 0);

  cbs_t v{f, imgui};
  return hooks_t{.callbacks = v, .cleanup = vaos.cleanup};
}

void process_events(uint64_t e, float delta) {
  if (e & event_t::increase_fov) {
    state.camera.update_fov(1.f);
  }
  if (e & event_t::decrease_fov) {
    state.camera.update_fov(-1.f);
  }
  if (e & event_t::camera_up) {
    state.camera.process_movement(CameraMovement::UP, delta);
  }
  if (e & event_t::camera_down) {
    state.camera.process_movement(CameraMovement::DOWN, delta);
  }
  if (e & event_t::camera_left) {
    state.camera.process_movement(CameraMovement::LEFT, delta);
  }
  if (e & event_t::camera_right) {
    state.camera.process_movement(CameraMovement::RIGHT, delta);
  }
  if (e & event_t::camera_for) {
    state.camera.process_movement(CameraMovement::FORWARD, delta);
  }
  if (e & event_t::camera_back) {
    state.camera.process_movement(CameraMovement::BACKWARD, delta);
  }
  if (e & event_t::camera_yaw_left) {
    state.camera.process_rotation(120 * -SPEED * delta, 0.f);
  }
  if (e & event_t::camera_yaw_right) {
    state.camera.process_rotation(120 * SPEED * delta, 0.f);
  }
}

buffers_t buffers() {
  id_t cube_vao;
  id_t vbo;
  glGenVertexArrays(1, &cube_vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(cube_vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, normal)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, texcoord)));
  glEnableVertexAttribArray(2);

  id_t light_vao;
  glGenVertexArrays(1, &light_vao);
  glBindVertexArray(light_vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, position)));
  glEnableVertexAttribArray(0);

  id_t pyramid_vao;
  id_t pyramid_vbo;
  id_t pyramid_ebo;
  glGenVertexArrays(1, &pyramid_vao);
  glGenBuffers(1, &pyramid_vbo);
  glGenBuffers(1, &pyramid_ebo);
  glBindVertexArray(pyramid_vao);

  glBindBuffer(GL_ARRAY_BUFFER, pyramid_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_vertices), pyramid_vertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramid_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramid_indices),
               pyramid_indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);

  auto cleanup = [cube_vao, light_vao, pyramid_vao, vbo, pyramid_vbo,
                  pyramid_ebo]() {
    glDeleteVertexArrays(1, &cube_vao);
    glDeleteVertexArrays(1, &light_vao);
    glDeleteVertexArrays(1, &pyramid_vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &pyramid_vbo);
    glDeleteBuffers(1, &pyramid_ebo);
  };

  return {
      .cube_vao = cube_vao,
      .light_vao = light_vao,
      .pyramid_vao = pyramid_vao,
      .cleanup = cleanup,
  };
}

void process_input(GLFWwindow *window, uint64_t &e);

struct delta_t {
  float last;  // Time of last frame
  float delta; // Time between current frame and last frame
};

void update_delta(delta_t &delta) {
  float now = glfwGetTime();
  delta.delta = now - delta.last;
  delta.last = now;
}

void event_loop(GLFWwindow *window,
                std::vector<std::function<void(uint64_t, float)>> cbs) {
  uint64_t e;

  delta_t delta{};

  while (!glfwWindowShouldClose(window)) {
    update_delta(delta);

    e = event_t::NONE;
    process_input(window, e);
    const preset_t &preset = presets[state.preset_index];
    glClearColor(preset.clear_color.x, preset.clear_color.y,
                 preset.clear_color.z, preset.clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto cb : cbs) {
      cb(e, delta.delta);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void process_input(GLFWwindow *window, uint64_t &e) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    e |= event_t::increase_fov;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    e |= event_t::decrease_fov;
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    e |= event_t::camera_up;
  }
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    e |= event_t::camera_down;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    e |= event_t::camera_left;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    e |= event_t::camera_right;
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    e |= event_t::camera_for;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    e |= event_t::camera_back;
  }
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    e |= event_t::camera_yaw_left;
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    e |= event_t::camera_yaw_right;
  }
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    e |= event_t::light_for;
  }
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    e |= event_t::light_back;
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    e |= event_t::light_left;
  }
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    e |= event_t::light_right;
  }
  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
    e |= event_t::light_up;
  }
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    e |= event_t::light_down;
  }
}
