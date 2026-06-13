#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;  

void main()
{ 
    vec2 offsets[25] = vec2[](
        vec2(-2*offset, 2*offset), // top-left
        vec2(-offset,   2*offset), // top-left
        vec2( 0.0f,     2*offset), // top-center
        vec2( offset,   2*offset), // top-right
        vec2( 2*offset, 2*offset), // top-right
        vec2(-2*offset, offset), // top-left
        vec2(-offset,   offset), // top-left
        vec2( 0.0f,     offset), // top-center
        vec2( offset,   offset), // top-right
        vec2( 2*offset, offset), // top-right
        vec2(-2*offset, 0.0f),   // center-left
        vec2(-offset,   0.0f),   // center-left
        vec2( 0.0f,     0.0f),   // center-center
        vec2( offset,   0.0f),   // center-right
        vec2( 2*offset, 0.0f),   // center-right
        vec2(-2*offset, -offset), // bottom-left
        vec2(-offset,   -offset), // bottom-left
        vec2( 0.0f,     -offset), // bottom-center
        vec2( offset,   -offset), // bottom-right    
        vec2( 2*offset, -offset), // bottom-right    
        vec2(-2*offset, -2*offset), // bottom-left
        vec2(-offset,   -2*offset), // bottom-left
        vec2( 0.0f,     -2*offset), // bottom-center
        vec2( offset,   -2*offset), // bottom-right    
        vec2( 2*offset, -2*offset)  // bottom-right    
    );

    float kernel[25] = float[](
        1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0,
        4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0 , 16.0 / 256.0, 4.0 / 256.0,
        6.0 / 256.0, 24.0 / 256.0, 36.0 / 256.0, 24.0 / 256.0, 6.0 / 256.0,
        4.0 / 256.0, 16.0 / 256.0, 24.0 / 256.0 , 16.0 / 256.0, 4.0 / 256.0,
        1.0 / 256.0, 4.0 / 256.0, 6.0 / 256.0, 4.0 / 256.0, 1.0 / 256.0
    );
    
    vec3 sampleTex[25];
    for(int i = 0; i < 25; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 25; i++)
        col += sampleTex[i] * kernel[i];
    
    FragColor = vec4(col, 1.0);
}
