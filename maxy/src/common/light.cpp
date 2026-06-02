#include <format>

#include "common/helpers.hpp"
#include "common/light.hpp"
#include "common/shader.hpp"

void set_light(id_t id, const std::string &name, const light_t &value) {
    set_vec3(id, std::format("{}.position", name), value.position);

    set_vec3(id, std::format("{}.ambient", name), value.ambient);
    set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
    set_vec3(id, std::format("{}.specular", name), value.specular);
}

void set_directional_light(
    id_t id, const std::string &name, const light_directional_t &value
) {
    set_vec3(id, std::format("{}.direction", name), value.direction);

    set_vec3(id, std::format("{}.ambient", name), value.ambient);
    set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
    set_vec3(id, std::format("{}.specular", name), value.specular);
}

void set_positional_light(
    id_t id, const std::string &name, const light_positional_t &value
) {
    set_vec3(id, std::format("{}.position", name), value.position);

    set_vec3(id, std::format("{}.ambient", name), value.ambient);
    set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
    set_vec3(id, std::format("{}.specular", name), value.specular);

    set_float(id, std::format("{}.constant", name), value.constant);
    set_float(id, std::format("{}.linear", name), value.linear);
    set_float(id, std::format("{}.quadratic", name), value.quadratic);
}

void set_spot_light(
    id_t id, const std::string &name, const light_spot_t &value
) {
    set_vec3(id, std::format("{}.position", name), value.position);
    set_vec3(id, std::format("{}.direction", name), value.direction);
    set_float(id, std::format("{}.cutoff", name), value.cutoff);
    set_float(id, std::format("{}.outer_cutoff", name), value.outer_cutoff);

    set_vec3(id, std::format("{}.ambient", name), value.ambient);
    set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
    set_vec3(id, std::format("{}.specular", name), value.specular);

    set_float(id, std::format("{}.constant", name), value.constant);
    set_float(id, std::format("{}.linear", name), value.linear);
    set_float(id, std::format("{}.quadratic", name), value.quadratic);
}

void set_flashlight(
    id_t id, const std::string &name, const flashlight_t &value
) {
    set_float(id, std::format("{}.cutoff", name), value.cutoff);
    set_float(id, std::format("{}.outer_cutoff", name), value.outer_cutoff);

    set_vec3(id, std::format("{}.ambient", name), value.ambient);
    set_vec3(id, std::format("{}.diffuse", name), value.diffuse);
    set_vec3(id, std::format("{}.specular", name), value.specular);

    set_float(id, std::format("{}.constant", name), value.constant);
    set_float(id, std::format("{}.linear", name), value.linear);
    set_float(id, std::format("{}.quadratic", name), value.quadratic);
}

light_positional_t random_positional_light() {
    glm::vec3 color{
        random_float(0.1f, 1.f), random_float(0.1f, 1.f),
        random_float(0.1f, 1.f)
    };
    return {
        .position =
            {random_float(-5.f, 5.f), random_float(-5.f, 5.f),
             random_float(-5.f, 5.f)},
        .ambient = color * 0.1f,
        .diffuse = color,
        .specular = color,
        .constant = 1.f,
        .linear = random_float(0.07f, 0.22f),
        .quadratic = random_float(0.017f, 0.07f),
    };
}

light_spot_t random_spot_light() {
    glm::vec3 color{
        random_float(0.1f, 1.f), random_float(0.1f, 1.f),
        random_float(0.1f, 1.f)
    };
    float cutoff_deg = random_float(10.f, 25.f);
    float outer_cutoff_deg = cutoff_deg + random_float(2.f, 8.f);
    glm::vec3 dir{
        random_float(-1.f, 1.f), random_float(-1.f, 1.f),
        random_float(-1.f, 1.f)
    };
    return {
        .position =
            {random_float(-5.f, 5.f), random_float(-5.f, 5.f),
             random_float(-5.f, 5.f)},
        .direction = glm::normalize(dir),
        .ambient = color * 0.05f,
        .diffuse = color,
        .specular = color,
        .cutoff = glm::cos(glm::radians(cutoff_deg)),
        .outer_cutoff = glm::cos(glm::radians(outer_cutoff_deg)),
        .constant = 1.f,
        .linear = random_float(0.07f, 0.22f),
        .quadratic = random_float(0.017f, 0.07f),
    };
}
