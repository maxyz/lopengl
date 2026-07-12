#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aModel;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

uniform float time;
uniform float orbitSpeed;

// Returns a rotation matrix around the Y axis for the given angle
mat4 rotationY(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat4(
        c,   0.0,  -s, 0.0,
        0.0, 1.0, 0.0, 0.0,
        s,   0.0,   c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}


void main()
{
    if (orbitSpeed != 0) {
        float angle = time * orbitSpeed;
        mat4 orbit = rotationY(angle);
        gl_Position = projection * view * orbit * aModel * vec4(aPos, 1.0); 
    } else {
        gl_Position = projection * view * aModel * vec4(aPos, 1.0); 
    }
    TexCoords = aTexCoords;
}
