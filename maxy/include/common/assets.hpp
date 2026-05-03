#pragma once

#include <expected>
#include <memory>

#include <glad/gl.h>

#include <stb/stb_image.h>

#include "common/types.hpp"

struct STBImageDeleter {
  void operator()(stbi_uc *ptr) const noexcept { stbi_image_free(ptr); }
};
using image_data_ptr = std::unique_ptr<stbi_uc[], STBImageDeleter>;

struct Image {
  image_data_ptr data;
  int width, height, channel_count;
};

std::expected<std::string, std::string>
get_asset_path(const std::string &filename);

std::expected<Image, std::string> load_image(const std::string &filename);

std::expected<id_t, std::string> load_texture(const std::string &filename);

std::expected<GLenum, std::string> guess_format(const Image &image);
