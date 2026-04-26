#include <format>

#include "common/materials.hpp"
#include "common/shader.hpp"

void set_material(id_t id, const std::string &name, const material_t &value) {
  set_vec3(id, std::format("{}.ambient", name), value.ambient);
  set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
  set_vec3(id, std::format("{}.specular", name), value.specular);
  set_float(id, std::format("{}.shininess", name), value.shininess);
}

void set_diffuse_map(id_t id, const std::string &name,
                     const diffuse_map_t &value) {
  set_int(id, std::format("{}.diffuse", name), value.diffuse);
  set_vec3(id, std::format("{}.specular", name), value.specular);
  set_float(id, std::format("{}.shininess", name), value.shininess);
}

void set_specular_map(id_t id, const std::string &name,
                      const specular_map_t &value) {
  set_int(id, std::format("{}.diffuse", name), value.diffuse);
  set_int(id, std::format("{}.specular", name), value.specular);
  set_float(id, std::format("{}.shininess", name), value.shininess);
}

void set_emission_map(id_t id, const std::string &name,
                      const emission_map_t &value) {
  set_int(id, std::format("{}.diffuse", name), value.diffuse);
  set_int(id, std::format("{}.specular", name), value.specular);
  set_int(id, std::format("{}.emission", name), value.emission);
  set_float(id, std::format("{}.shininess", name), value.shininess);
}
