#version 330 core
out vec4 FragColor;
uniform vec3 leftColor;
uniform vec3 rightColor;
uniform float halfScreen;

void main()
{
    if(gl_FragCoord.x < halfScreen)
        FragColor = vec4(leftColor, 1.0);
    else
        FragColor = vec4(rightColor, 1.0);   

}
