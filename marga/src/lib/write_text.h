#ifndef WRITE_TEXT_H
#define WRITE_TEXT_H

#include <string>
#include <vector>
#include <fstream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_truetype.h"
#include "shader.h"


class TextWriter {

    private:
        // Internal texture id
        GLuint textureID;
        // Internal VAO, VBO
        GLuint textVAO, textVBO;
        // Internal character data
        stbtt_bakedchar cdata[96];
        // Size of the character atlas
        int ATLAS_W, ATLAS_H;

    public:
        TextWriter(std::string font_filename);
        void write(std::string message);
};

#endif  
