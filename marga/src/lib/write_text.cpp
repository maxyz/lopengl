#include "write_text.h"


TextWriter::TextWriter(std::string font_filename)
{
    // Load font file into memory
    std::vector<unsigned char> fontBuffer;
    std::ifstream file(font_filename, std::ios::binary);
    fontBuffer.assign(std::istreambuf_iterator<char>(file), {});

    // Bake a 512x512 bitmap atlas for ASCII chars 32–127
    ATLAS_W = 512, ATLAS_H = 512;
    std::vector<unsigned char> atlasBitmap(ATLAS_W * ATLAS_H);
    // cdata contains 96 chars starting at ASCII 32
    stbtt_BakeFontBitmap(fontBuffer.data(), 0,
        32.0f,                  // font size in pixels
        atlasBitmap.data(), ATLAS_W, ATLAS_H,
        32, 96,                 // first char, num chars
        cdata);

    // Upload atlas to a GL texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
        ATLAS_W, ATLAS_H, 0,
        GL_RED, GL_UNSIGNED_BYTE, atlasBitmap.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // textVAO and textVBO are instance attributes as they get used for writing
    glGenVertexArrays(1, &textVAO);
    glBindVertexArray(textVAO);

    glGenBuffers(1, &textVBO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void TextWriter::write(std::string message)
{

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    
    float x = 10.0f;
    float y = 10.0f; // 10px from the bottom
    
    for (const char* p = message.c_str(); *p; p++) {
        if (*p < 32 || *p >= 128) continue;
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(cdata, ATLAS_W, ATLAS_H,
            *p - 32,  // char index
            &x, &y,   // position, updated per glyph
            &q, 1);   // 1 = opengl y-direction
    
        // q gives two triangles forming the glyph quad
        float verts[6][4] = {
            { q.x0, -q.y1,  q.s0, q.t1 },
            { q.x0, -q.y0,  q.s0, q.t0 },
            { q.x1, -q.y0,  q.s1, q.t0 },
    
            { q.x0, -q.y1,  q.s0, q.t1 },
            { q.x1, -q.y0,  q.s1, q.t0 },
            { q.x1, -q.y1,  q.s1, q.t1 },
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}



