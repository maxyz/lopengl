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
#include "engine.h"

class Shader
{
public:

    // The program ID
    GLuint program;

    // Constructor reads and builds the shader
    Shader() = default;
    Shader(const std::string vertexPath, const std::string fragmentPath);
    Shader(const std::string vertexPath, const std::string fragmentPath, const std::string geometryPath);
    Shader& operator=(const Shader& other) {
        program = other.program;
        return *this;
    }

    // Use the program
    void use();
    void setInt(const std::string uniformName, const int value);
    void setBool(const std::string uniformName, const bool value);
    void setFloat(const std::string uniformName, const float value);
    void setVec3(const std::string uniformName, const glm::vec3 &value);
    void setVec4(const std::string uniformName, const glm::vec4 &value);
    void setMat3(const std::string uniformName, const glm::mat3 &value);
    void setMat4(const std::string uniformName, const glm::mat4 &value);

    void setVertexMatrices(glm::mat4 &view, glm::mat4 &model, glm::mat4 &projection);

private:
    GLuint compileShader(std::string shaderPath, uint shaderType);
};

#endif