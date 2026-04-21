#include <format>

#include "common/light.hpp"

void set_light(id_t id, const std::string &name, const light_t &value) {
  set_vec3(id, std::format("{}.position", name), value.position);

  set_vec3(id, std::format("{}.ambient", name), value.ambient);
  set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
  set_vec3(id, std::format("{}.specular", name), value.specular);
}
