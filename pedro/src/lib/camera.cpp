#include "camera.h"

glm::mat4 myLookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    
    glm::mat4 rotation(1.0f), traslation(1.0f);
    glm::vec3 zAxis = glm::normalize(position - target);
    glm::vec3 xAxis = glm::normalize(glm::cross(glm::normalize(up), zAxis));
    glm::vec3 yAxis = glm::cross(zAxis, xAxis);

    rotation[0][0] = xAxis.x;
    rotation[1][0] = xAxis.y;
    rotation[2][0] = xAxis.z;

    rotation[0][1] = yAxis.x;
    rotation[1][1] = yAxis.y;
    rotation[2][1] = yAxis.z;

    rotation[0][2] = zAxis.x;
    rotation[1][2] = zAxis.y;
    rotation[2][2] = zAxis.z;

    traslation[3][0] = - position.x;
    traslation[3][1] = - position.y;
    traslation[3][2] = - position.z;

    return rotation * traslation;

    /*    // 1. Position = known
    // 2. Calculate cameraDirection
    glm::vec3 zaxis = glm::normalize(position - target);
    // 3. Get positive right axis vector
    glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
    // 4. Calculate camera up vector
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // Create translation and rotation matrix
    // In glm we access elements as mat[col][row] due to column-major layout
    glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
    translation[3][0] = -position.x; // Fourth column, first row
    translation[3][1] = -position.y;
    translation[3][2] = -position.z;
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = xaxis.x; // First column, first row
    rotation[1][0] = xaxis.y;
    rotation[2][0] = xaxis.z;
    rotation[0][1] = yaxis.x; // First column, second row
    rotation[1][1] = yaxis.y;
    rotation[2][1] = yaxis.z;
    rotation[0][2] = zaxis.x; // First column, third row
    rotation[1][2] = zaxis.y;
    rotation[2][2] = zaxis.z; 

    // Return lookAt matrix as combination of translation and rotation matrix
    return rotation * translation; */
}

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
    if (myCamMode) {
        return myLookAt(position, position + front, worldUp);
    } else {
        return glm::lookAt(position, position + front, worldUp);
    }
    
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

