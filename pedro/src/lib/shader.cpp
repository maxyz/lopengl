#include "shader.h"

Shader::Shader(const std::string vertexPath, const std::string fragmentPath) 
{
    GLuint vertex   = compileShader(vertexPath, GL_VERTEX_SHADER),
           fragment = compileShader(fragmentPath, GL_FRAGMENT_SHADER);        

    // Shader Program
    this->program = glCreateProgram();
    glAttachShader(this->program, vertex);
    glAttachShader(this->program, fragment);
    glLinkProgram(this->program);
    
    GLint success;
    GLchar infoLog[512];
    // Print linking errors if any
    glGetProgramiv(this->program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(this->program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::Shader(const std::string vertexPath, const std::string fragmentPath, const std::string geometryPath)
{
    GLuint vertex   = compileShader(vertexPath, GL_VERTEX_SHADER),
           fragment = compileShader(fragmentPath, GL_FRAGMENT_SHADER),
           geometry = compileShader(geometryPath, GL_GEOMETRY_SHADER);     

    // Shader Program
    this->program = glCreateProgram();
    glAttachShader(this->program, vertex);
    glAttachShader(this->program, fragment);
    glAttachShader(this->program, geometry);
    glLinkProgram(this->program);

    GLint success;
    GLchar infoLog[512];
    // Print linking errors if any
    glGetProgramiv(this->program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(this->program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteShader(geometry);
}


GLuint Shader::compileShader(std::string shaderPath, uint shaderType) 
{    
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string shaderCodeString;
    std::ifstream shaderFile;

    // ensures ifstream objects can throw exceptions:
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        // Open files
        shaderFile.open(shaderPath);
        std::stringstream vShaderStream, fShaderStream;

        // Read file’s buffer contents into streams
        vShaderStream << shaderFile.rdbuf();

        // close file handlers
        shaderFile.close();

        // Convert stream into GLchar array
        shaderCodeString = vShaderStream.str();
    }

    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ_AT: " << shaderPath << std::endl;
    }

    const GLchar* shaderCode = shaderCodeString.c_str();

    // 2. Compile shaders
    GLuint shader;
    GLint success;
    GLchar infoLog[512];

    // Vertex Shader
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);

    // Print compile errors if any
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    return shader;
}


void Shader::use() {
    glUseProgram(this->program);
}

void Shader::setBool(const std::string uniformName, const bool value) {
    glUniform1i(glGetUniformLocation(this->program, uniformName.c_str()), (int)value);
}

void Shader::setInt(const std::string uniformName, const int value) {
    glUniform1i(glGetUniformLocation(this->program, uniformName.c_str()), value);
}

void Shader::setFloat(const std::string uniformName, const float value) {
    glUniform1f(glGetUniformLocation(this->program, uniformName.c_str()), value);
}

void Shader::setVec3(const std::string uniformName, const glm::vec3 &value) {
    glUniform3fv(glGetUniformLocation(this->program, uniformName.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string uniformName, const glm::vec4 &value) {
    glUniform4fv(glGetUniformLocation(this->program, uniformName.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string uniformName, const glm::mat3 &value) {
    glUniformMatrix4fv(glGetUniformLocation(this->program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string uniformName, const glm::mat4 &value) {
    glUniformMatrix4fv(glGetUniformLocation(this->program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVertexMatrices(glm::mat4 &view, glm::mat4 &model, glm::mat4 &projection) {
    setMat4("view", view);
    setMat4("model", model);
    setMat4("projection", projection);
}