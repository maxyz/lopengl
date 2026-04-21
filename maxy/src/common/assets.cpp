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
  auto path = get_asset_path(filename);
  if (!path) {
    return std::unexpected(path.error());
  }
  stbi_set_flip_vertically_on_load(true);
  Image image;
  auto data = stbi_load((*path).c_str(), &image.width, &image.height,
                        &image.nr_channels, 0);
  if (!data) {
    return std::unexpected(
        std::format("Failed to load texture ({})", filename));
  }
  image.data.reset(data);
  return image;
}

std::expected<id_t, std::string> load_texture(const std::string &filename) {
  auto image = load_image(filename);
  if (!image) {
    return std::unexpected(image.error());
  }
  auto format = guess_format(*image);
  if (!format) {
    return std::unexpected(format.error());
  }

  id_t texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0,
               *format, GL_UNSIGNED_BYTE, image->data.get());
  glGenerateMipmap(GL_TEXTURE_2D);

  return texture;
}

std::expected<GLenum, std::string> guess_format(const Image &image) {
  if (image.nr_channels == 1)
    return GL_RED;
  if (image.nr_channels == 3)
    return GL_RGB;
  if (image.nr_channels == 4)
    return GL_RGBA;
  return std::unexpected(std::format(
      "Could not detect image format, invalid amount of channels {}",
      image.nr_channels));
}
