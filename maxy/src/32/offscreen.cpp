#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Anti aliassing in framebuffers";
constexpr GLuint      WIDTH  = 1024;
constexpr GLuint      HEIGHT = 768;

struct state_t {
    window_state_t window;
};

state_t state{
    .window = {
        .viewport = {.width = WIDTH, .height = HEIGHT},
        .camera   = Camera{glm::vec3(0.f, 0.f, 3.f)},
    }
};

struct shaders_t {
    Shader buffer;
    Shader screen;
};

struct vaos_t {
    id_t cube;
    id_t screen;
};
struct vbos_t {
    id_t cube;
    id_t screen;
};
struct textures_t {
    id_t multi_sample;
    id_t screen;
};
struct framebuffers_t {
    id_t screen;
    id_t intermediate;
};
struct renderbuffers_t {
    id_t screen;
};

struct vertex_2d_tex_t {
    glm::vec2 position;
    glm::vec2 tex_coord;
};

inline const std::array<vertex_2d_tex_t, 6> quad_vertices = {{
    {{-1.f, 1.f}, {0.f, 1.f}},
    {{-1.f, -1.f}, {0.f, 0.f}},
    {{1.f, -1.f}, {1.f, 0.f}},

    {{-1.f, 1.f}, {0.f, 1.f}},
    {{1.f, -1.f}, {1.f, 0.f}},
    {{1.f, 1.f}, {1.f, 1.f}},
}};

class SceneRenderer {
public:
    static std::expected<std::unique_ptr<SceneRenderer>, std::string> create(GLFWwindow *window);

    SceneRenderer(const SceneRenderer &)            = delete;
    SceneRenderer &operator=(const SceneRenderer &) = delete;
    SceneRenderer(SceneRenderer &&o) noexcept       = delete;
    SceneRenderer &operator=(SceneRenderer &&o)     = delete;

    ~SceneRenderer() noexcept {
        glDeleteVertexArrays(sizeof(m_vaos) / sizeof(id_t), reinterpret_cast<id_t *>(&m_vaos));
        glDeleteBuffers(sizeof(m_vbos) / sizeof(id_t), reinterpret_cast<id_t *>(&m_vbos));

        glDeleteRenderbuffers(
            sizeof(m_renderbuffers) / sizeof(id_t), reinterpret_cast<id_t *>(&m_renderbuffers)
        );
        glDeleteFramebuffers(
            sizeof(m_framebuffers) / sizeof(id_t), reinterpret_cast<id_t *>(&m_framebuffers)
        );
    }

    void render(input_t input, float delta);

    void on_window_resize(int width, int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers.screen);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textures.multi_sample);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
        glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffers.screen);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            // Handle framebuffer generation error here
            // We will probably need to add an error flag, as we are in an event
            // handler here.
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers.intermediate);
        glBindTexture(GL_TEXTURE_2D, m_textures.screen);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffers.screen);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            // Handle framebuffer generation error here
            // We will probably need to add an error flag, as we are in an event
            // handler here.
        }

        // Unbind assets to maintain clean state
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    shaders_t       m_shaders;
    vaos_t          m_vaos;
    vbos_t          m_vbos;
    framebuffers_t  m_framebuffers;
    renderbuffers_t m_renderbuffers;
    textures_t      m_textures;

    SceneRenderer(
        shaders_t shaders, vaos_t vaos, vbos_t vbos, framebuffers_t framebuffers,
        renderbuffers_t renderbuffers, textures_t textures
    )
        : m_shaders{std::move(shaders)}, m_vaos{vaos}, m_vbos{vbos}, m_framebuffers{framebuffers},
          m_renderbuffers{renderbuffers}, m_textures{textures} {}
};

std::expected<shaders_t, std::string> load_shaders() {
    auto buffer_shader = Shader::build("shaders/32_buffer.vert", "shaders/32_buffer.frag");
    if (!buffer_shader) return std::unexpected(buffer_shader.error());

    auto screen_shader = Shader::build("shaders/32_screen.vert", "shaders/32_screen.frag");
    if (!screen_shader) return std::unexpected(screen_shader.error());

    return shaders_t{.buffer = std::move(*buffer_shader), .screen = std::move(*screen_shader)};
}

void load_quad_vertices(id_t vertex_array_object, id_t vertex_buffer_object) {
    // Target for the framebuffer rendering
    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_2d_tex_t),
        reinterpret_cast<void *>(offsetof(vertex_2d_tex_t, position))
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_2d_tex_t),
        reinterpret_cast<void *>(offsetof(vertex_2d_tex_t, tex_coord))
    );
    glEnableVertexAttribArray(1);
}

std::pair<vaos_t, vbos_t> load_buffers() {
    vaos_t vaos{};
    vbos_t vbos{};

    glGenVertexArrays(sizeof(vaos_t) / sizeof(id_t), reinterpret_cast<id_t *>(&vaos));
    glGenBuffers(sizeof(vbos_t) / sizeof(id_t), reinterpret_cast<id_t *>(&vbos));

    buffers::load_vertices(cube_vertices, vaos.cube, vbos.cube);

    load_quad_vertices(vaos.screen, vbos.screen);

    return {vaos, vbos};
}

std::expected<std::tuple<framebuffers_t, renderbuffers_t, textures_t>, std::string>
create_framebuffers() {
    framebuffers_t  framebuffers;
    renderbuffers_t renderbuffers;
    textures_t      textures;
    glGenFramebuffers(
        sizeof(framebuffers_t) / sizeof(id_t), reinterpret_cast<id_t *>(&framebuffers)
    );
    glGenRenderbuffers(
        sizeof(renderbuffers_t) / sizeof(id_t), reinterpret_cast<id_t *>(&renderbuffers)
    );
    glGenTextures(sizeof(textures_t) / sizeof(id_t), reinterpret_cast<id_t *>(&textures));

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.screen);

    // create a multisampled color attachment texture
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textures.multi_sample);
    glTexImage2DMultisample(
        GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, state.window.viewport.width,
        state.window.viewport.height, GL_TRUE
    );
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textures.multi_sample, 0
    );

    // create a render buffer object for depth and stencil attachment
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers.screen);
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, state.window.viewport.width,
        state.window.viewport.height
    );
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffers.screen
    );

    // now that we actually created the framebuffer and added all attachments we
    // want to check if it is actually complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteRenderbuffers(
            sizeof(renderbuffers_t) / sizeof(id_t), reinterpret_cast<id_t *>(&renderbuffers)
        );
        glDeleteFramebuffers(
            sizeof(framebuffers_t) / sizeof(id_t), reinterpret_cast<id_t *>(&framebuffers)
        );
        return std::unexpected("Failed to complete framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.intermediate);
    glBindTexture(GL_TEXTURE_2D, textures.screen);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, state.window.viewport.width, state.window.viewport.height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures.screen, 0
    ); // we only need a color buffer

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteRenderbuffers(
            sizeof(renderbuffers_t) / sizeof(id_t), reinterpret_cast<id_t *>(&renderbuffers)
        );
        glDeleteFramebuffers(
            sizeof(framebuffers_t) / sizeof(id_t), reinterpret_cast<id_t *>(&framebuffers)
        );
        return std::unexpected("Failed to complete framebuffer");
    }

    return std::tuple{framebuffers, renderbuffers, textures};
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
    auto shaders = load_shaders();
    if (!shaders) return std::unexpected(shaders.error());

    auto [vaos, vbos] = load_buffers();

    auto create_framebuffers_res = create_framebuffers();
    if (!create_framebuffers_res) return std::unexpected(create_framebuffers_res.error());
    auto [framebuffers, renderbuffers, textures] = *create_framebuffers_res;

    shaders->screen.use();
    shaders->screen.set_int("screen_texture", 0);

    auto renderer =
        new SceneRenderer{std::move(*shaders), vaos, vbos, framebuffers, renderbuffers, textures};
    return std::unique_ptr<SceneRenderer>{renderer};
}

void SceneRenderer::render(input_t input, float delta) {
    process_camera_events(state.window, input, delta);

    // 1. draw scene as normal in multisampled buffers
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers.screen);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 view       = state.window.camera.get_view_matrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(state.window.camera.fov),
        static_cast<float>(state.window.viewport.width) /
            static_cast<float>(state.window.viewport.height),
        .1f, 1000.f
    );
    glm::mat4 model_transform = glm::mat4(1.f);

    m_shaders.buffer.use();
    m_shaders.buffer.set_mat4("view", view);
    m_shaders.buffer.set_mat4("projection", projection);
    m_shaders.buffer.set_mat4("model", model_transform);

    glBindVertexArray(m_vaos.cube);
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

    // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored
    // in screenTexture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffers.screen);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffers.intermediate);
    glBlitFramebuffer(
        0, 0, state.window.viewport.width, state.window.viewport.height, 0, 0,
        state.window.viewport.width, state.window.viewport.height, GL_COLOR_BUFFER_BIT, GL_NEAREST
    );

    // 3. now render quad with scene's visuals as its texture image
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // draw Screen quad
    m_shaders.screen.use();
    glBindVertexArray(m_vaos.screen);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(
        GL_TEXTURE_2D, m_textures.screen
    ); // use the now resolved color attachment as the quad's texture
    glDrawArrays(GL_TRIANGLES, 0, quad_vertices.size());
}

int error_exit(std::string error) {
    std::cerr << error << "\n";
    return -1;
}

int main() {
    auto ctx = GLContext::create(WIDTH, HEIGHT, TITLE);
    if (!ctx) return error_exit(ctx.error());

    init_window_callbacks(ctx->window(), state.window);

    auto expected_renderer = SceneRenderer::create(ctx->window());
    if (!expected_renderer) return error_exit(expected_renderer.error());
    auto renderer                   = std::move(*expected_renderer);
    auto renderer_ptr               = renderer.get();
    state.window.on_resize_callback = [renderer_ptr](int width, int height) {
        renderer_ptr->on_window_resize(width, height);
    };

    event_loop(ctx->window(), *renderer, process_common_input);
    return 0;
}
