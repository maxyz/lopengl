#include <expected>
#include <filesystem>
#include <format>
#include <string>

#include "common/assets.hpp"
#include <stb/stb_image.h>

namespace fs = std::filesystem;

std::expected<std::string, std::string>
get_asset_path(const std::string &filename) {
#ifdef ASSETS_PATH
  fs::path assetsPath{ASSETS_PATH};
  auto filepath = assetsPath / filename;
  if (fs::exists(filepath))
    return filepath;
#endif

  return std::unexpected(std::format("Could not find asset ({})", filename));
}

std::expected<Image, std::string> load_image(const std::string &filename) {
  stbi_set_flip_vertically_on_load(true);
  return get_asset_path(filename).and_then(
      [&](const std::string &path) -> std::expected<Image, std::string> {
        Image image;
        auto *data = stbi_load(path.c_str(), &image.width, &image.height,
                               &image.channel_count, 0);
        if (!data)
          return std::unexpected(
              std::format("Failed to load texture ({})", filename));
        image.data.reset(data);
        return image;
      });
}

static std::expected<std::pair<Image, GLenum>, std::string>
with_format(Image image) {
  return guess_format(image).transform(
      [img = std::move(image)](GLenum fmt) mutable {
        return std::pair{std::move(img), fmt};
      });
}

std::expected<id_t, std::string> load_texture(const std::string &filename) {
  return load_image(filename)
      .and_then(with_format)
      .transform([](std::pair<Image, GLenum> p) {
        auto [image, format] = std::move(p);
        id_t texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0,
                     format, GL_UNSIGNED_BYTE, image.data.get());
        glGenerateMipmap(GL_TEXTURE_2D);
        return texture;
      });
}

std::expected<id_t, std::string> load_texture(const std::string &filename,
                                              const std::string &directory) {
  fs::path path{directory};
  auto filepath = path / filename;
  return load_texture(filepath);
}

std::expected<GLenum, std::string> guess_format(const Image &image) {
  if (image.channel_count == 1)
    return GL_RED;
  if (image.channel_count == 3)
    return GL_RGB;
  if (image.channel_count == 4)
    return GL_RGBA;
  return std::unexpected(std::format(
      "Could not detect image format, invalid amount of channels {}",
      image.channel_count));
}

