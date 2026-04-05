#ifndef SHADER_H
#define SHADER_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h> 

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
};

#endif