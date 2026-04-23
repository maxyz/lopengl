#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    // This seems to work ok
    //Normal = vec3(model * vec4(aNormal, 0.0));
    // This is what the tutorial says. It's relevant when doing non-uniform scaling
    Normal = mat3(transpose(inverse(model))) * aNormal;
}
