#pragma once

#include <string>

#include <glm/glm.hpp>

#include "common/shader.hpp"

struct light_t {
  glm::vec3 position;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
};

void set_light(id_t id, const std::string &name, const light_t &value);
