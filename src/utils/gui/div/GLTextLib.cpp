#include <iostream>

#include "DrawLib.h"


DrawLib::DrawLib() :    
    VertexArrayObjects(0), 
    VertexBufferObjects(0) {
    // declare freetype
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    } else {
	    // declare font
        const std::string font_name = "D:/SUMOMaterial/projects/FreeType_example/arial.ttf";
	    // load font as face
        FT_Face face;
        if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        } else {
            // set size to load glyphs as
            FT_Set_Pixel_Sizes(face, 0, 48);
            // disable byte-alignment restriction
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            // load first 128 characters of ASCII set
            for (unsigned char c = 0; c < 128; c++) {
                // Load character glyph 
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                    std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                } else {
                    // generate texture
                    unsigned int texture;
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RED,
                        face->glyph->bitmap.width,
                        face->glyph->bitmap.rows,
                        0,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        face->glyph->bitmap.buffer
                    );
                    // set texture options
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    // now store character for later use
                    DrawLib::Character character = {
                        texture,
                        face->glyph->bitmap.width, face->glyph->bitmap.rows,
                        face->glyph->bitmap_left, face->glyph->bitmap_top,
                        static_cast<unsigned int>(face->glyph->advance.x)
                    };
                    Characters.insert(std::pair<char, DrawLib::Character>(c, character));
                }
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }
}


void 
DrawLib::RenderText(Shader &shader, std::string text, float x, float y, float scale, float R, float G, float B) {
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), R, G, B);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VertexArrayObjects);
    // iterate through all characters
    for (const auto &c : text) {
        const DrawLib::Character ch = Characters[c];
        const float xpos = x + ch.bitmap_left * scale;
        const float ypos = y - (ch.rows - ch.bitmap_top) * scale;
        const float w = ch.width * scale;
        const float h = ch.rows * scale;
        // update VBO for each character
        const float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObjects);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}