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

struct light_directional_t {
  glm::vec3 direction;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
};

struct light_positional_t {
  glm::vec3 position;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct light_spot_t {
  glm::vec3 position;
  glm::vec3 direction;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;

  float cutoff;
  float outer_cutoff;

  float constant;
  float linear;
  float quadratic;
};

void set_light(id_t id, const std::string &name, const light_t &value);
void set_directional_light(id_t id, const std::string &name,
                           const light_directional_t &value);
void set_positional_light(id_t id, const std::string &name,
                          const light_positional_t &value);
void set_spot_light(id_t id, const std::string &name,
                    const light_spot_t &value);
