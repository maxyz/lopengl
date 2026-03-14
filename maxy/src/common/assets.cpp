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
