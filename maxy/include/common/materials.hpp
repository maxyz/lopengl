#pragma once

#include <string>

#include <glm/glm.hpp>

#include "common/shader.hpp"

struct material_t {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
};

struct diffuse_map_t {
  id_t diffuse;

  glm::vec3 specular;
  float shininess;
};

struct specular_map_t {
  id_t diffuse;
  id_t specular;
  float shininess;
};

struct emission_map_t {
  id_t diffuse;
  id_t specular;
  id_t emission;
  float shininess;
};

void set_material(id_t id, const std::string &name, const material_t &value);
void set_diffuse_map(id_t id, const std::string &name,
                     const diffuse_map_t &value);
void set_specular_map(id_t id, const std::string &name,
                      const specular_map_t &value);
void set_emission_map(id_t id, const std::string &name,
                      const emission_map_t &value);

const material_t emerald = {
    .ambient = glm::vec3(0.0215, 0.1745, 0.0215),
    .diffuse = glm::vec3(0.07568, 0.61424, 0.07568),
    .specular = glm::vec3(0.633, 0.727811, 0.633),
    .shininess = 0.6 * 128,
};

const material_t jade = {
    .ambient = glm::vec3(0.135, 0.2225, 0.1575),
    .diffuse = glm::vec3(0.54, 0.89, 0.63),
    .specular = glm::vec3(0.316228, 0.316228, 0.316228),
    .shininess = 0.1 * 128,
};

const material_t obsidian = {
    .ambient = glm::vec3(0.05375, 0.05, 0.06625),
    .diffuse = glm::vec3(0.18275, 0.17, 0.22525),
    .specular = glm::vec3(0.332741, 0.328634, 0.346435),
    .shininess = 0.3 * 128,
};

const material_t pearl = {
    .ambient = glm::vec3(0.25, 0.20725, 0.20725),
    .diffuse = glm::vec3(1, 0.829, 0.829),
    .specular = glm::vec3(0.296648, 0.296648, 0.296648),
    .shininess = 0.088 * 128,
};

const material_t ruby = {
    .ambient = glm::vec3(0.1745, 0.01175, 0.01175),
    .diffuse = glm::vec3(0.61424, 0.04136, 0.04136),
    .specular = glm::vec3(0.727811, 0.626959, 0.626959),
    .shininess = 0.6 * 128,
};

const material_t turquoise = {
    .ambient = glm::vec3(0.1, 0.18725, 0.1745),
    .diffuse = glm::vec3(0.396, 0.74151, 0.69102),
    .specular = glm::vec3(0.297254, 0.30829, 0.306678),
    .shininess = 0.1 * 128,
};

const material_t brass = {
    .ambient = glm::vec3(0.329412, 0.223529, 0.027451),
    .diffuse = glm::vec3(0.780392, 0.568627, 0.113725),
    .specular = glm::vec3(0.992157, 0.941176, 0.807843),
    .shininess = 0.21794872 * 128,
};

const material_t bronze = {
    .ambient = glm::vec3(0.2125, 0.1275, 0.054),
    .diffuse = glm::vec3(0.714, 0.4284, 0.18144),
    .specular = glm::vec3(0.393548, 0.271906, 0.166721),
    .shininess = 0.2 * 128,
};

const material_t chrome = {
    .ambient = glm::vec3(0.25, 0.25, 0.25),
    .diffuse = glm::vec3(0.4, 0.4, 0.4),
    .specular = glm::vec3(0.774597, 0.774597, 0.774597),
    .shininess = 0.6 * 128,
};

const material_t copper = {
    .ambient = glm::vec3(0.19125, 0.0735, 0.0225),
    .diffuse = glm::vec3(0.7038, 0.27048, 0.0828),
    .specular = glm::vec3(0.256777, 0.137622, 0.086014),
    .shininess = 0.1 * 128,
};

const material_t gold = {
    .ambient = glm::vec3(0.24725, 0.1995, 0.0745),
    .diffuse = glm::vec3(0.75164, 0.60648, 0.22648),
    .specular = glm::vec3(0.628281, 0.555802, 0.366065),
    .shininess = 0.4 * 128,
};

const material_t silver = {
    .ambient = glm::vec3(0.19225, 0.19225, 0.19225),
    .diffuse = glm::vec3(0.50754, 0.50754, 0.50754),
    .specular = glm::vec3(0.508273, 0.508273, 0.508273),
    .shininess = 0.4 * 128,
};

const material_t black_plastic = {
    .ambient = glm::vec3(0.0, 0.0, 0.0),
    .diffuse = glm::vec3(0.01, 0.01, 0.01),
    .specular = glm::vec3(0.50, 0.50, 0.50),
    .shininess = 0.25 * 128,
};

const material_t cyan_plastic = {
    .ambient = glm::vec3(0.0, 0.1, 0.06),
    .diffuse = glm::vec3(0.0, 0.50980392, 0.50980392),
    .specular = glm::vec3(0.50196078, 0.50196078, 0.50196078),
    .shininess = 0.25 * 128,
};

const material_t green_plastic = {
    .ambient = glm::vec3(0.0, 0.0, 0.0),
    .diffuse = glm::vec3(0.1, 0.35, 0.1),
    .specular = glm::vec3(0.45, 0.55, 0.45),
    .shininess = 0.25 * 128,
};

const material_t red_plastic = {
    .ambient = glm::vec3(0.0, 0.0, 0.0),
    .diffuse = glm::vec3(0.5, 0.0, 0.0),
    .specular = glm::vec3(0.7, 0.6, 0.6),
    .shininess = 0.25 * 128,
};

const material_t white_plastic = {
    .ambient = glm::vec3(0.0, 0.0, 0.0),
    .diffuse = glm::vec3(0.55, 0.55, 0.55),
    .specular = glm::vec3(0.70, 0.70, 0.70),
    .shininess = 0.25 * 128,
};

const material_t yellow_plastic = {
    .ambient = glm::vec3(0.0, 0.0, 0.0),
    .diffuse = glm::vec3(0.5, 0.5, 0.0),
    .specular = glm::vec3(0.60, 0.60, 0.50),
    .shininess = 0.25 * 128,
};

const material_t black_rubber = {
    .ambient = glm::vec3(0.02, 0.02, 0.02),
    .diffuse = glm::vec3(0.01, 0.01, 0.01),
    .specular = glm::vec3(0.4, 0.4, 0.4),
    .shininess = 0.078125 * 128,
};

const material_t cyan_rubber = {
    .ambient = glm::vec3(0.0, 0.05, 0.05),
    .diffuse = glm::vec3(0.4, 0.5, 0.5),
    .specular = glm::vec3(0.04, 0.7, 0.7),
    .shininess = .078125 * 128,
};

const material_t green_rubber = {
    .ambient = glm::vec3(0.0, 0.05, 0.0),
    .diffuse = glm::vec3(0.4, 0.5, 0.4),
    .specular = glm::vec3(0.04, 0.7, 0.04),
    .shininess = 0.078125,
};

const material_t red_rubber = {
    .ambient = glm::vec3(0.05, 0.0, 0.0),
    .diffuse = glm::vec3(0.5, 0.4, 0.4),
    .specular = glm::vec3(0.7, 0.04, 0.04),
    .shininess = .078125 * 128,
};

const material_t white_rubber = {
    .ambient = glm::vec3(0.05, 0.05, 0.05),
    .diffuse = glm::vec3(0.5, 0.5, 0.5),
    .specular = glm::vec3(0.7, 0.7, 0.7),
    .shininess = .078125 * 128,
};

const material_t yellow_rubber = {
    .ambient = glm::vec3(0.05, 0.05, 0.0),
    .diffuse = glm::vec3(0.5, 0.5, 0.4),
    .specular = glm::vec3(0.7, 0.7, 0.04),
    .shininess = .078125 * 128,
};
