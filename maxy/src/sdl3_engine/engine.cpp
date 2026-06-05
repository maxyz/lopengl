#include "engine.hpp"
#include "geometry.hpp"

#include <format>
#include <fstream>
#include <utility>
#include <vector>

#include <SDL3_image/SDL_image.h>

std::unexpected<std::string> sdl_error(std::string prefix) {
    return std::unexpected(std::format("{}: {}", prefix, SDL_GetError()));
}

namespace {

std::expected<gpu_buffer_t, std::string> create_buffer(
    engine_t const &engine, SDL_GPUBufferUsageFlags usage, void const *data, Uint32 size
) {
    using transfer_t = gpu_resource_t<SDL_GPUTransferBuffer, SDL_ReleaseGPUTransferBuffer>;

    SDL_GPUBufferCreateInfo buffer_info = {};
    buffer_info.usage                   = usage;
    buffer_info.size                    = size;
    gpu_buffer_t buffer{engine.gpu_device, SDL_CreateGPUBuffer(engine.gpu_device, &buffer_info)};
    if (!buffer) return sdl_error("SDL_CreateGPUBuffer failed");

    SDL_GPUTransferBufferCreateInfo transfer_info = {};
    transfer_info.usage                           = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size                            = size;
    transfer_t transfer{
        engine.gpu_device, SDL_CreateGPUTransferBuffer(engine.gpu_device, &transfer_info)
    };
    if (!transfer) return sdl_error("SDL_CreateGPUTransferBuffer failed");

    void *mapped = SDL_MapGPUTransferBuffer(engine.gpu_device, transfer.get(), false);
    if (!mapped) return sdl_error("SDL_MapGPUTransferBuffer failed");
    SDL_memcpy(mapped, data, size);
    SDL_UnmapGPUTransferBuffer(engine.gpu_device, transfer.get());

    SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!cmd_buf) return sdl_error("SDL_AcquireGPUCommandBuffer failed");

    SDL_GPUTransferBufferLocation source = {};
    source.transfer_buffer               = transfer.get();

    SDL_GPUBufferRegion destination = {};
    destination.buffer              = buffer.get();
    destination.size                = size;

    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);
    SDL_EndGPUCopyPass(copy_pass);

    if (!SDL_SubmitGPUCommandBuffer(cmd_buf)) return sdl_error("SDL_SubmitGPUCommandBuffer failed");

    return buffer;
}

} // namespace

engine_t::engine_t(engine_t &&other) noexcept
    : window(std::exchange(other.window, nullptr)),
      gpu_device(std::exchange(other.gpu_device, nullptr)),
      sdl_initialized(std::exchange(other.sdl_initialized, false)), verbose(other.verbose),
      last_tick(other.last_tick) {}

engine_t &engine_t::operator=(engine_t &&other) noexcept {
    if (this != &other) {
        this->~engine_t();
        window          = std::exchange(other.window, nullptr);
        gpu_device      = std::exchange(other.gpu_device, nullptr);
        sdl_initialized = std::exchange(other.sdl_initialized, false);
        verbose         = other.verbose;
        last_tick       = other.last_tick;
    }
    return *this;
}

engine_t::~engine_t() {
    if (gpu_device && window) SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
    if (gpu_device) SDL_DestroyGPUDevice(gpu_device);
    if (window) SDL_DestroyWindow(window);
    if (sdl_initialized) SDL_Quit();
}

engine_config_t parse_engine_args(int argc, char *argv[]) {
    engine_config_t config;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--verbose") config.verbose = true;
    }
    return config;
}

std::expected<engine_t, std::string>
create_engine(std::string_view title, int width, int height, engine_config_t const &config) {
    engine_t engine;
    engine.verbose = config.verbose;

    if (!SDL_Init(SDL_INIT_VIDEO)) return sdl_error("SDL_Init failed");
    engine.sdl_initialized = true;

    engine.window = SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_RESIZABLE);
    if (!engine.window) return sdl_error("SDL_CreateWindow failed");

    engine.gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
    if (!engine.gpu_device) return sdl_error("SDL_CreateGPUDevice failed");

    if (!SDL_ClaimWindowForGPUDevice(engine.gpu_device, engine.window))
        return sdl_error("SDL_ClaimWindowForGPUDevice failed");

    if (engine.verbose) SDL_Log("GPU driver: %s", SDL_GetGPUDeviceDriver(engine.gpu_device));
    return engine;
}

bool poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) return false;
    }
    return true;
}

float tick(engine_t &engine) {
    Uint64 now       = SDL_GetTicks();
    float  dt        = engine.last_tick == 0 ? 0.0f : (now - engine.last_tick) / 1000.0f;
    engine.last_tick = now;
    return dt;
}

glm::ivec2 window_pixel_size(engine_t const &engine) {
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(engine.window, &w, &h);
    return {w, h};
}

float aspect_ratio(engine_t const &engine) {
    auto size = window_pixel_size(engine);
    return float(size.x) / float(size.y);
}

std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color,
    std::function<void(SDL_GPUCommandBuffer *, SDL_GPURenderPass *)> draw
) {
    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!command_buffer) return sdl_error("SDL_AcquireGPUCommandBuffer failed");

    SDL_GPUTexture *swapchain_texture = nullptr;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, engine.window, &swapchain_texture, nullptr, nullptr
        )) {
        SDL_CancelGPUCommandBuffer(command_buffer);
        return sdl_error("SDL_WaitAndAcquireGPUSwapchainTexture failed");
    }

    if (swapchain_texture) {
        SDL_GPUColorTargetInfo target = {};
        target.texture                = swapchain_texture;
        target.clear_color            = clear_color;
        target.load_op                = SDL_GPU_LOADOP_CLEAR;
        target.store_op               = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass *render_pass =
            SDL_BeginGPURenderPass(command_buffer, &target, 1, nullptr);
        draw(command_buffer, render_pass);
        SDL_EndGPURenderPass(render_pass);
    }

    if (!SDL_SubmitGPUCommandBuffer(command_buffer))
        return sdl_error("SDL_SubmitGPUCommandBuffer failed");

    return {};
}

std::expected<gpu_shader_t, std::string> load_shader(
    engine_t const &engine, std::string_view spv_path, SDL_GPUShaderStage stage,
    Uint32 num_uniform_buffers, Uint32 num_samplers
) {
    std::ifstream file(spv_path.data(), std::ios::binary | std::ios::ate);
    if (!file) return std::unexpected(std::format("Cannot open shader: {}", spv_path));

    auto size = file.tellg();
    file.seekg(0);
    std::vector<Uint8> code(size);
    file.read(reinterpret_cast<char *>(code.data()), size);
    if (!file) return std::unexpected(std::format("Failed to read shader: {}", spv_path));

    SDL_GPUShaderCreateInfo info = {};
    info.code_size               = code.size();
    info.code                    = code.data();
    info.entrypoint              = "main";
    info.format                  = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage                   = stage;
    info.num_uniform_buffers     = num_uniform_buffers;
    info.num_samplers            = num_samplers;

    SDL_GPUShader *shader = SDL_CreateGPUShader(engine.gpu_device, &info);
    if (!shader) return sdl_error("SDL_CreateGPUShader failed");
    return gpu_shader_t{engine.gpu_device, shader};
}

std::expected<gpu_buffer_t, std::string>
create_vertex_buffer(engine_t const &engine, void const *data, Uint32 size) {
    return create_buffer(engine, SDL_GPU_BUFFERUSAGE_VERTEX, data, size);
}

std::expected<gpu_buffer_t, std::string>
create_index_buffer(engine_t const &engine, void const *data, Uint32 size) {
    return create_buffer(engine, SDL_GPU_BUFFERUSAGE_INDEX, data, size);
}

std::expected<gpu_pipeline_t, std::string>
create_pipeline(engine_t const &engine, pipeline_desc_t const &desc) {
    auto vert = load_shader(
        engine, desc.vertex_shader, SDL_GPU_SHADERSTAGE_VERTEX, desc.vertex_uniform_buffers
    );
    if (!vert) return std::unexpected(vert.error());

    auto frag = load_shader(
        engine, desc.fragment_shader, SDL_GPU_SHADERSTAGE_FRAGMENT, desc.fragment_uniform_buffers,
        desc.fragment_samplers
    );
    if (!frag) return std::unexpected(frag.error());

    // Default vertex layout: one vertex_t (float3 position) at location 0.
    SDL_GPUVertexBufferDescription default_buffer_desc = {
        .slot = 0, .pitch = sizeof(vertex_t), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
    };
    SDL_GPUVertexAttribute default_attribute = {
        .location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0
    };
    auto buffer_descs = desc.vertex_buffer_descs.empty() ? std::span{&default_buffer_desc, 1}
                                                         : desc.vertex_buffer_descs;
    auto attrs =
        desc.vertex_attributes.empty() ? std::span{&default_attribute, 1} : desc.vertex_attributes;

    SDL_GPUColorTargetDescription color_target = {};
    color_target.format = SDL_GetGPUSwapchainTextureFormat(engine.gpu_device, engine.window);

    SDL_GPUGraphicsPipelineCreateInfo info             = {};
    info.vertex_shader                                 = vert->get();
    info.fragment_shader                               = frag->get();
    info.vertex_input_state.vertex_buffer_descriptions = buffer_descs.data();
    info.vertex_input_state.num_vertex_buffers         = static_cast<Uint32>(buffer_descs.size());
    info.vertex_input_state.vertex_attributes          = attrs.data();
    info.vertex_input_state.num_vertex_attributes      = static_cast<Uint32>(attrs.size());
    info.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.target_info.color_target_descriptions         = &color_target;
    info.target_info.num_color_targets                 = 1;
    if (desc.enable_depth_test) {
        info.depth_stencil_state.compare_op        = SDL_GPU_COMPAREOP_LESS;
        info.depth_stencil_state.enable_depth_test  = true;
        info.depth_stencil_state.enable_depth_write = true;
        info.target_info.depth_stencil_format       = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
        info.target_info.has_depth_stencil_target   = true;
    }

    gpu_pipeline_t pipeline{
        engine.gpu_device, SDL_CreateGPUGraphicsPipeline(engine.gpu_device, &info)
    };
    if (!pipeline) return sdl_error("SDL_CreateGPUGraphicsPipeline failed");
    return pipeline;
}

std::expected<gpu_texture_t, std::string>
load_texture(engine_t const &engine, std::string_view path) {
    using transfer_t = gpu_resource_t<SDL_GPUTransferBuffer, SDL_ReleaseGPUTransferBuffer>;

    SDL_Surface *raw = IMG_Load(path.data());
    if (!raw) return std::unexpected(std::format("IMG_Load failed: {}", SDL_GetError()));

    SDL_Surface *rgba = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(raw);
    if (!rgba) return sdl_error("SDL_ConvertSurface failed");

    // SDL3_GPU uses top-left UV origin; flip vertically so images appear
    // the same way as in OpenGL (which loaded images bottom-row-first).
    if (!SDL_FlipSurface(rgba, SDL_FLIP_VERTICAL)) {
        SDL_DestroySurface(rgba);
        return sdl_error("SDL_FlipSurface failed");
    }

    Uint32 const data_size = rgba->w * rgba->h * 4;

    SDL_GPUTextureCreateInfo tex_info = {};
    tex_info.type                     = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format                   = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage                    = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    tex_info.width                    = rgba->w;
    tex_info.height                   = rgba->h;
    tex_info.layer_count_or_depth     = 1;
    tex_info.num_levels               = 1;

    gpu_texture_t texture{engine.gpu_device, SDL_CreateGPUTexture(engine.gpu_device, &tex_info)};
    if (!texture) {
        SDL_DestroySurface(rgba);
        return sdl_error("SDL_CreateGPUTexture failed");
    }

    SDL_GPUTransferBufferCreateInfo transfer_info = {};
    transfer_info.usage                           = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size                            = data_size;
    transfer_t transfer{
        engine.gpu_device, SDL_CreateGPUTransferBuffer(engine.gpu_device, &transfer_info)
    };
    if (!transfer) {
        SDL_DestroySurface(rgba);
        return sdl_error("SDL_CreateGPUTransferBuffer failed");
    }

    void *mapped = SDL_MapGPUTransferBuffer(engine.gpu_device, transfer.get(), false);
    if (!mapped) {
        SDL_DestroySurface(rgba);
        return sdl_error("SDL_MapGPUTransferBuffer failed");
    }
    SDL_memcpy(mapped, rgba->pixels, data_size);
    SDL_UnmapGPUTransferBuffer(engine.gpu_device, transfer.get());
    SDL_DestroySurface(rgba);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!cmd) return sdl_error("SDL_AcquireGPUCommandBuffer failed");

    SDL_GPUTextureTransferInfo source = {};
    source.transfer_buffer            = transfer.get();
    source.pixels_per_row             = tex_info.width;
    source.rows_per_layer             = tex_info.height;

    SDL_GPUTextureRegion destination = {};
    destination.texture              = texture.get();
    destination.w                    = tex_info.width;
    destination.h                    = tex_info.height;
    destination.d                    = 1;

    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmd);
    SDL_UploadToGPUTexture(copy_pass, &source, &destination, false);
    SDL_EndGPUCopyPass(copy_pass);

    if (!SDL_SubmitGPUCommandBuffer(cmd)) return sdl_error("SDL_SubmitGPUCommandBuffer failed");

    return texture;
}

std::expected<gpu_sampler_t, std::string> create_sampler(
    engine_t const &engine, SDL_GPUSamplerAddressMode address_mode, SDL_GPUFilter filter
) {
    SDL_GPUSamplerCreateInfo info = {};
    info.min_filter               = filter;
    info.mag_filter               = filter;
    info.mipmap_mode              = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    info.address_mode_u           = address_mode;
    info.address_mode_v           = address_mode;
    info.address_mode_w           = address_mode;

    SDL_GPUSampler *sampler = SDL_CreateGPUSampler(engine.gpu_device, &info);
    if (!sampler) return sdl_error("SDL_CreateGPUSampler failed");
    return gpu_sampler_t{engine.gpu_device, sampler};
}

std::expected<gpu_geometry_t, std::string> create_geometry(
    engine_t const &engine, void const *vertices, Uint32 vertex_size,
    std::span<uint16_t const> indices
) {
    auto vertex_buffer = create_buffer(engine, SDL_GPU_BUFFERUSAGE_VERTEX, vertices, vertex_size);
    if (!vertex_buffer) return std::unexpected(vertex_buffer.error());

    Uint32 index_size = static_cast<Uint32>(indices.size() * sizeof(uint16_t));
    auto   index_buffer =
        create_buffer(engine, SDL_GPU_BUFFERUSAGE_INDEX, indices.data(), index_size);
    if (!index_buffer) return std::unexpected(index_buffer.error());

    return gpu_geometry_t{
        std::move(*vertex_buffer), std::move(*index_buffer), static_cast<Uint32>(indices.size())
    };
}

std::expected<gpu_geometry_t, std::string>
create_vertex_geometry(engine_t const &engine, void const *vertices, Uint32 vertex_size,
                       Uint32 vertex_count) {
    auto vertex_buffer = create_buffer(engine, SDL_GPU_BUFFERUSAGE_VERTEX, vertices, vertex_size);
    if (!vertex_buffer) return std::unexpected(vertex_buffer.error());
    gpu_geometry_t g;
g.vertex_buffer = std::move(*vertex_buffer);
g.vertex_count  = vertex_count;
return g;
}

std::expected<gpu_material_t, std::string>
create_material(engine_t const &engine, material_desc_t desc) {
    std::vector<gpu_texture_t> textures;
    textures.reserve(desc.texture_paths.size());
    for (auto const &path : desc.texture_paths) {
        auto tex = load_texture(engine, path);
        if (!tex) return std::unexpected(tex.error());
        textures.push_back(std::move(*tex));
    }

    std::vector<gpu_sampler_t> samplers;
    samplers.reserve(desc.texture_paths.size());
    for (size_t i = 0; i < desc.texture_paths.size(); ++i) {
        auto mode   = i < desc.address_modes.size() ? desc.address_modes[i]
                                                    : SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
        auto filter = i < desc.filter_modes.size() ? desc.filter_modes[i] : SDL_GPU_FILTER_LINEAR;
        auto s      = create_sampler(engine, mode, filter);
        if (!s) return std::unexpected(s.error());
        samplers.push_back(std::move(*s));
    }

    return gpu_material_t{std::move(textures), std::move(samplers)};
}

void draw(
    gpu_pipeline_t const &pipeline, gpu_geometry_t const &geometry, gpu_material_t const &material,
    SDL_GPURenderPass *pass
) {
    SDL_BindGPUGraphicsPipeline(pass, pipeline.get());

    SDL_GPUBufferBinding vbinding = {geometry.vertex_buffer.get(), 0};
    SDL_BindGPUVertexBuffers(pass, 0, &vbinding, 1);

    if (geometry.index_count > 0) {
        SDL_GPUBufferBinding ibinding = {geometry.index_buffer.get(), 0};
        SDL_BindGPUIndexBuffer(pass, &ibinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    std::vector<SDL_GPUTextureSamplerBinding> bindings;
    bindings.reserve(material.textures.size());
    for (size_t i = 0; i < material.textures.size(); ++i)
        bindings.push_back({material.textures[i].get(), material.samplers[i].get()});
    SDL_BindGPUFragmentSamplers(pass, 0, bindings.data(), static_cast<Uint32>(bindings.size()));

    if (geometry.index_count > 0)
        SDL_DrawGPUIndexedPrimitives(pass, geometry.index_count, 1, 0, 0, 0);
    else
        SDL_DrawGPUPrimitives(pass, geometry.vertex_count, 1, 0, 0);
}

std::expected<gpu_texture_t, std::string>
create_depth_texture(engine_t const &engine, int width, int height) {
    SDL_GPUTextureCreateInfo info = {};
    info.type                 = SDL_GPU_TEXTURETYPE_2D;
    info.format               = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    info.usage                = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    info.width                = static_cast<Uint32>(width);
    info.height               = static_cast<Uint32>(height);
    info.layer_count_or_depth = 1;
    info.num_levels           = 1;

    SDL_GPUTexture *tex = SDL_CreateGPUTexture(engine.gpu_device, &info);
    if (!tex) return sdl_error("SDL_CreateGPUTexture (depth) failed");
    return gpu_texture_t{engine.gpu_device, tex};
}

bool tracked_depth_t::update(engine_t const &engine) {
    auto current = window_pixel_size(engine);
    if (current == size) return true;
    auto result = create_depth_texture(engine, current.x, current.y);
    if (!result) return false;
    texture = std::move(*result);
    size    = current;
    return true;
}

std::expected<tracked_depth_t, std::string>
create_tracked_depth(engine_t const &engine) {
    auto size   = window_pixel_size(engine);
    auto result = create_depth_texture(engine, size.x, size.y);
    if (!result) return std::unexpected(result.error());
    return tracked_depth_t{std::move(*result), size};
}

std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color,
    gpu_texture_t const &depth_texture,
    std::function<void(SDL_GPUCommandBuffer *, SDL_GPURenderPass *)> draw) {
    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!command_buffer) return sdl_error("SDL_AcquireGPUCommandBuffer failed");

    SDL_GPUTexture *swapchain_texture = nullptr;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, engine.window, &swapchain_texture, nullptr, nullptr
        )) {
        SDL_CancelGPUCommandBuffer(command_buffer);
        return sdl_error("SDL_WaitAndAcquireGPUSwapchainTexture failed");
    }

    if (swapchain_texture) {
        SDL_GPUColorTargetInfo color = {};
        color.texture     = swapchain_texture;
        color.clear_color = clear_color;
        color.load_op     = SDL_GPU_LOADOP_CLEAR;
        color.store_op    = SDL_GPU_STOREOP_STORE;

        SDL_GPUDepthStencilTargetInfo depth = {};
        depth.texture           = depth_texture.get();
        depth.clear_depth       = 1.0f;
        depth.load_op           = SDL_GPU_LOADOP_CLEAR;
        depth.store_op          = SDL_GPU_STOREOP_DONT_CARE;
        depth.stencil_load_op   = SDL_GPU_LOADOP_DONT_CARE;
        depth.stencil_store_op  = SDL_GPU_STOREOP_DONT_CARE;
        depth.cycle             = true;

        SDL_GPURenderPass *render_pass =
            SDL_BeginGPURenderPass(command_buffer, &color, 1, &depth);
        draw(command_buffer, render_pass);
        SDL_EndGPURenderPass(render_pass);
    }

    if (!SDL_SubmitGPUCommandBuffer(command_buffer))
        return sdl_error("SDL_SubmitGPUCommandBuffer failed");

    return {};
}
