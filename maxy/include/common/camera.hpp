#pragma once

#include <algorithm>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN,
};

constexpr float camera_speed = 2.5f;
constexpr float camera_sensitivity = 0.1f;
constexpr float camera_fov = 45.f;
constexpr float camera_default_yaw = -90.f;
constexpr float camera_pitch_limit = 89.f;
constexpr float camera_min_fov = 1.f;
constexpr float camera_max_fov = 90.f;

class Camera {
public:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 world_up;
  float yaw;
  float pitch;
  float fov = camera_fov;
  float speed = camera_speed;
  float sensitivity = camera_sensitivity;

  Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
         float yaw = camera_default_yaw, float pitch = 0.f)
      : position(position), yaw(yaw), pitch(pitch),
        world_up(glm::vec3(0.f, 1.f, 0.f)) {
    update_vectors();
  }

  bool fly() { return m_fly; }
  void fly(const bool fly) { m_fly = fly; }

  glm::mat4 get_view_matrix() {
    return glm::lookAt(position, position + front, up);
  }

  void process_movement(CameraMovement direction, float delta) {
    float velocity = speed * delta;
    glm::vec3 forward_direction = front;
    if (!m_fly) {
      forward_direction = glm::cross(world_up, right);
    }
    switch (direction) {
    case FORWARD:
      position += forward_direction * velocity;
      break;
    case BACKWARD:
      position -= forward_direction * velocity;
      break;
    case LEFT:
      position -= right * velocity;
      break;
    case RIGHT:
      position += right * velocity;
      break;
    case UP:
      position += up * velocity;
      break;
    case DOWN:
      position -= up * velocity;
      break;
    }
  }

  void process_rotation(float x_offset, float y_offset,
                        bool constraint_pitch = true) {
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    if (constraint_pitch) {
      pitch = std::clamp(pitch, -camera_pitch_limit, camera_pitch_limit);
    }
    update_vectors();
  }

  void update_fov(float delta) { fov = std::clamp(fov + delta, camera_min_fov, camera_max_fov); }

private:
  bool m_fly = true;

  void update_vectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
  }
};
