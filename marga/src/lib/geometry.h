#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

void createVertexBuffers(unsigned int *VAO, unsigned int *VBO, const void *data, unsigned int size) {
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glBindVertexArray(*VAO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void setVertexAttribs(unsigned int first, unsigned int second, unsigned int third)
{
    unsigned int total = first + second + third;
    glVertexAttribPointer(0, first, GL_FLOAT, GL_FALSE, total * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    if (second == 0) return;
    glVertexAttribPointer(1, second, GL_FLOAT, GL_FALSE, total * sizeof(float), (void*)(first * sizeof(float)));
    glEnableVertexAttribArray(1);
    if (third == 0) return;
    glVertexAttribPointer(2, third, GL_FLOAT, GL_FALSE, total * sizeof(float), (void*)(first+second * sizeof(float)));
    glEnableVertexAttribArray(2);
}


const float cubeVertices[] = {
    // positions          // normals           // texture coords
    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // bottom-right 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // top-left
    // Front face                                                          
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, // bottom-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f, // top-right
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f, // top-right
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f, // top-left
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
    // Left face                                                           
    -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
    -0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-left
    -0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-left
    -0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-right
    -0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-right
    // Right face                                                          
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top-right    
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // bottom-right
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // top-left
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom-left  
    // Bottom face                                                         
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // top-left
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // bottom-left
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // bottom-right
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // top-right
    // Top face                                                            
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // top-right    
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // bottom-right
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top-left
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f  // bottom-left  
};

void getCubeBuffers(unsigned int *cubeVAO, unsigned int *cubeVBO) {
    createVertexBuffers(cubeVAO, cubeVBO, &cubeVertices, sizeof(cubeVertices));
    setVertexAttribs(3,3,2);
}

void getLightBuffers(unsigned int *lightVAO, unsigned int cubeVBO) {
    // The lights use the same cubeVBO pre-initialized but create their own VAO
    glGenVertexArrays(1, lightVAO);
    glBindVertexArray(*lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // set the vertex attribute
    setVertexAttribs(3,0,0);
}


// Plane definition
const float planeVertices[] = {
    // positions          normals           texture Coords
    // Note we set the texture higher than 1 (together with GL_REPEAT as texture wrapping mode).
    // this will cause the floor texture to repeat)
     5.0f, -0.6f,  5.0f,  0.0f, 1.0f, 0.0f, 2.0f, 0.0f,
    -5.0f, -0.6f, -5.0f,  0.0f, 1.0f, 0.0f, 0.0f, 2.0f,
    -5.0f, -0.6f,  5.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

     5.0f, -0.6f,  5.0f,  0.0f, 1.0f, 0.0f, 2.0f, 0.0f,
     5.0f, -0.6f, -5.0f,  0.0f, 1.0f, 0.0f, 2.0f, 2.0f,
    -5.0f, -0.6f, -5.0f,  0.0f, 1.0f, 0.0f, 0.0f, 2.0f
};

void getPlaneBuffers(unsigned int *planeVAO, unsigned int *planeVBO) {
    createVertexBuffers(planeVAO, planeVBO, &planeVertices, sizeof(planeVertices));
    setVertexAttribs(3,3,2);
}


// vertex attributes for a rectangle that fills the entire screen in Normalized Device Coordinates.
const float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

void getQuadBuffers(unsigned int *quadVAO, unsigned int *quadVBO) {
    createVertexBuffers(quadVAO, quadVBO, &quadVertices, sizeof(quadVertices));
    setVertexAttribs(2,2,0);
}

const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

void getSkyboxBuffers(unsigned int *skyboxVAO, unsigned int *skyboxVBO) {
    createVertexBuffers(skyboxVAO, skyboxVBO, &skyboxVertices, sizeof(skyboxVertices));
    setVertexAttribs(3,0,0);
}

float points[] = {
    -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
    -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
}; 

void getPointBuffers(unsigned int *pointsVAO, unsigned int *pointsVBO) {
    createVertexBuffers(pointsVAO, pointsVBO, &points, sizeof(points));
    setVertexAttribs(2,3,0);
}


#endif
