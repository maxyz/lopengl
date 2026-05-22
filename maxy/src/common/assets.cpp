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
  auto asset_path = get_asset_path(filename);
  if (!asset_path) {
    return std::unexpected(asset_path.error());
  }
  Image image;
  auto *data = stbi_load(
      asset_path->c_str(), &image.width, &image.height, &image.channel_count, 0
  );
  if (!data)
    return std::unexpected(
        std::format("Failed to load texture ({})", filename)
    );
  image.data.reset(data);
  return image;
}

std::expected<id_t, std::string> load_texture(const std::string &filename) {
  return load_texture(filename, DEFAULT_TEXTURE_OPTIONS);
}

std::expected<id_t, std::string>
load_texture(const std::string &filename, const texture_options_t &options) {
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrap);
  glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(
      GL_TEXTURE_2D, 0, *format, image->width, image->height, 0, *format,
      GL_UNSIGNED_BYTE, image->data.get()
  );
  glGenerateMipmap(GL_TEXTURE_2D);
  return texture;
}

std::expected<id_t, std::string>
load_texture(const std::string &filename, const std::string &directory) {
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
  return std::unexpected(
      std::format(
          "Could not detect image format, invalid amount of channels {}",
          image.channel_count
      )
  );
}
