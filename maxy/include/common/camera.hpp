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
  float yaw;
  float pitch;
  float fov = camera_fov;
  float speed = camera_speed;
  float sensitivity = camera_sensitivity;

  Camera(glm::vec3 position = glm::vec3(0.f, 0.f, 0.f),
         float yaw = camera_default_yaw, float pitch = 0.f)
      : position(position), yaw(yaw), pitch(pitch),
        m_world_up(glm::vec3(0.f, 1.f, 0.f)) {
    update_vectors();
  }

  bool is_flying() const { return m_fly; }
  void fly(const bool fly) { m_fly = fly; }

  const glm::vec3 &front() const { return m_front; }
  const glm::vec3 &right() const { return m_right; }
  const glm::vec3 &up() const { return m_up; }

  glm::mat4 get_view_matrix() const {
    return glm::lookAt(position, position + m_front, m_up);
  }

  void process_movement(CameraMovement direction, float delta) {
    float velocity = speed * delta;
    glm::vec3 forward_direction = m_front;
    if (!m_fly) {
      forward_direction = glm::cross(m_world_up, m_right);
    }
    switch (direction) {
    case FORWARD:
      position += forward_direction * velocity;
      break;
    case BACKWARD:
      position -= forward_direction * velocity;
      break;
    case LEFT:
      position -= m_right * velocity;
      break;
    case RIGHT:
      position += m_right * velocity;
      break;
    case UP:
      position += m_up * velocity;
      break;
    case DOWN:
      position -= m_up * velocity;
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
  glm::vec3 m_front;
  glm::vec3 m_right;
  glm::vec3 m_up;
  glm::vec3 m_world_up;
  bool m_fly = true;

  void update_vectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    m_front = glm::normalize(f);
    m_right = glm::normalize(glm::cross(m_front, m_world_up));
    m_up = glm::normalize(glm::cross(m_right, m_front));
  }
};
