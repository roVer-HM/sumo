#pragma once
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "GLShader.h"

class DrawLib {
    
public:
    /// @brief draw lib
    DrawLib();

    /// @brief render text
    void RenderText(Shader &shader, std::string text, float x, float y, float scale, float R, float G, float B);

    unsigned int VertexArrayObjects, VertexBufferObjects;

private:
    /// @brief Holds all state information relevant to a character as loaded using FreeType
    struct Character {
        /// @brief ID handle of the glyph texture
        unsigned int TextureID; 
        /// @brief Size of glyph
        unsigned int width;
        unsigned int rows;
        /// @brief Offset from baseline to left/top of glyph
        FT_Int bitmap_left;
        FT_Int bitmap_top;
        /// @brief Horizontal offset to advance to next glyph
        unsigned int Advance;   
    };
    /// @brief characters
    std::map<char, Character> Characters;
};