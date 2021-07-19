#pragma once

#include <string>


class Shader {

public:
    /// @brief constructor generates the shader on the fly
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

    /// @brief activate the shader
    void use();

    /// @brief shader ID
    unsigned int ID;

private:
    /// @brief utility function for checking shader compilation/linking errors.
    void checkCompileErrors(unsigned int shader, std::string type);
};