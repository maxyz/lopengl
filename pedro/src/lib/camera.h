#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum CameraMovement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    WORLD_UP,
    DOWN
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float FOV        =  45.0f;

class Camera {

public:

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 worldUp;
    glm::vec3 right;

    float speed;
    float deltaSpeed;
    float fov;
    float pitch;
    float yaw;
    float mouseSensitivity;

    bool myCamMode;;

    Camera(glm::vec3 inPosition = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 inUp = glm::vec3(0.0f, 1.0f, 0.0f), float inYaw = YAW, float inPitch = PITCH) :
    front(glm::vec3(0.0f, 0.0f, -1.0f)), 
    speed(SPEED), 
    mouseSensitivity(SENSITIVITY),
    fov(FOV),
    myCamMode(false)
    {   
        position = inPosition;
        worldUp = inUp;
        up = worldUp;
        yaw = inYaw;
        pitch = inPitch;
        updateCameraVectors();
    }

    void moveLeft();
    void moveRight();
    void moveFront();
    void moveFrontPlane();
    void moveBack();
    void moveBackPlane();
    void moveUp();
    void moveWorldUp();
    void moveDown();
    void moveWorldDown();

    void handleMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void handleMouseScroll(float yoffset);

    glm::mat4 lookFront();

    void resetFov() { fov = FOV; };

private:
    
    void updateCameraVectors();

};

#endif