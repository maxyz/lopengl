#include "camera.h"

void Camera::moveLeft(){
    position -= right * deltaSpeed;
}

void Camera::moveRight(){
    position += right * deltaSpeed;
}

void Camera::moveFront() {
    position += deltaSpeed * front;
}

void Camera::moveFrontPlane() {
    position += deltaSpeed * glm::normalize(glm::vec3(front.x, 0.0f, front.z));
}

void Camera::moveBack() {
    position -= deltaSpeed * front;
}

void Camera::moveBackPlane() {
    position -= deltaSpeed * glm::normalize(glm::vec3(front.x, 0.0f, front.z));
}

void Camera::moveUp() {
    position += deltaSpeed * up;
}

void Camera::moveWorldUp() {
    position += deltaSpeed * worldUp;
}

void Camera::moveDown() {
    position -= deltaSpeed * up;
}

void Camera::moveWorldDown() {
    position -= deltaSpeed * worldUp;
}

glm::mat4 Camera::lookFront() {
    return glm::lookAt(position, position + front, worldUp);
}

void Camera::handleMouseMovement(float xoffset, float yoffset, bool constrainPitch){
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if (constrainPitch)
    {
        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::handleMouseScroll(float yoffset){
        fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f; 
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    right = glm::normalize(glm::cross(front, worldUp));  
    up    = glm::normalize(glm::cross(right, front));
}