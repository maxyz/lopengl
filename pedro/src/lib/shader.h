#ifndef SHADER_H
#define SHADER_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:

    // The program ID
    GLuint Program;

    // Constructor reads and builds the shader
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

    // Use the program
    void Use();
    void setInt(const GLchar* uniformName, const int value);
    void setBool(const GLchar* uniformName, const bool value);
    void setVec3(const GLchar* uniformName, const glm::vec3 &value);
    void setVec4(const GLchar* uniformName, const glm::vec4 &value);
    void setMat4(const GLchar* uniformName, const glm::mat4 &value);
};

#endif