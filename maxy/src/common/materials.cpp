#include <format>

#include "common/materials.hpp"
#include "common/shader.hpp"

void set_material(id_t id, const std::string &name, const material_t &value) {
  set_vec3(id, std::format("{}.ambient", name), value.ambient);
  set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
  set_vec3(id, std::format("{}.specular", name), value.specular);
  set_float(id, std::format("{}.shininess", name), value.shininess);
}
