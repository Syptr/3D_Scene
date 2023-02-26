/*Content originated from website LearnOpenGL.com, which distributes the code
with the following information regarding licensing:
All code samples, unless explicitly stated otherwise, are licensed under the terms
of the CC BY-NC 4.0 license as published by Creative Commons, either version 4 of
the License, or (at your option) any later version. You can find a human-readable format of the license
https://creativecommons.org/licenses/by-nc/4.0/
and the full license
https://creativecommons.org/licenses/by-nc/4.0/legalcode
*/

//*Modified code to include movement speed change via scroll wheel, and Q, E key functions. - SJ**

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Abstraction to define options for camera movement despite system specific input methods.
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    DOWN,
    UP
};

//Set default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


//Abstract camera class to process device input & calculate corresponding euler angles, vectors, and matrices.
class Camera
{
public:
    //Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    //Euler Angles
    float Yaw;
    float Pitch;

    //Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    //Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    //Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    //Return the view matrix calculated using Euler Angles & the LookAt matrix
    glm::mat4 GetViewMatrix() const {return glm::lookAt(Position, Position + Front, Up);}

    //Process input from keyboard like devices 
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
        if (direction == UP)
            Position += Up * velocity;
    }

    //Process input received from mouse input device. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Constrain pitch so screen doesn't flip
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // Process the input from the mouse scroll event allowing the user to increase/decrease movement speed
    void ProcessMouseScroll(float yoffset)
    {
        //Scrolling up increases speed while scrolling down decreases speed
        MovementSpeed += (float)yoffset;
        if (MovementSpeed < 0.10f) //minimum speed
            MovementSpeed = 0.10f;
        if (MovementSpeed > 12.0f) //max speed
            MovementSpeed = 12.0f;
    }

private:
    //Calculate the front vector from the Camera's new Euler Angles
    void updateCameraVectors()
    {
        //Calculate new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        //Re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  //Normalize vectors to prevent slower movement as length gets closer to 0
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif
