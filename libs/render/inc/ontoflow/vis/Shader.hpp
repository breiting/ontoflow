#pragma once
#include <glad.h>

#include <glm/glm.hpp>
#include <string>

namespace of::vis {

/**
 * \brief Represents an OpenGL shader program.
 *
 * This class handles the loading, compilation, linking, and activation of
 * OpenGL vertex and fragment shaders. It also provides methods to set
 * uniform variables within the shader program.
 */
class Shader {
   public:
    /**
     * \brief Constructs a Shader object by compiling and linking vertex and fragment shader source codes.
     * \param vertexSource The source code for the vertex shader.
     * \param fragmentSource The source code for the fragment shader.
     */
    Shader(const std::string& vertexSource, const std::string& fragmentSource);

    /**
     * \brief Destructor. Deletes the OpenGL shader program.
     */
    ~Shader();

    /**
     * \brief Activates this shader program for rendering.
     */
    void Bind();

    /**
     * \brief Sets a mat4 uniform in the shader.
     * \param name The name of the uniform variable in the shader.
     * \param matrix The glm::mat4 matrix to set.
     */
    void SetMat4(const std::string& name, const glm::mat4& matrix);

    /**
     * \brief Sets a vec2 uniform in the shader.
     * \param name The name of the uniform variable in the shader.
     * \param value The glm::vec2 vector to set.
     */
    void SetVec2(const std::string& name, const glm::vec2& value);

    /**
     * \brief Sets a vec3 uniform in the shader.
     * \param name The name of the uniform variable in the shader.
     * \param value The glm::vec3 vector to set.
     */
    void SetVec3(const std::string& name, const glm::vec3& value);

    /**
     * \brief Sets a vec4 uniform in the shader.
     * \param name The name of the uniform variable in the shader.
     * \param value The glm::vec4 vector to set.
     */
    void SetVec4(const std::string& name, const glm::vec4& value);

    /**
     * \brief Sets a float uniform in the shader.
     * \param name The name of the uniform variable in the shader.
     * \param value The float value to set.
     */
    void SetFloat(const std::string& name, float value);

    /**
     * \brief Sets a boolean uniform in the shader.
     * Internally sets an integer uniform (0 for false, 1 for true).
     * \param name The name of the uniform variable in the shader.
     * \param value The boolean value to set.
     */
    void SetBool(const std::string& name, bool value);

    /**
     * \brief Gets the location of a uniform variable in the shader program.
     * \param name The name of the uniform variable.
     * \return The uniform location ID.
     */
    int GetUniformLocation(const std::string& name) const;

   private:
    unsigned int m_ID; ///< The OpenGL program ID.

    /**
     * \brief Reads the content of a shader file into a string.
     * \param path The file path to the shader source.
     * \return The content of the shader file as a string.
     */
    std::string readShaderFile(const std::string& path);

    /**
     * \brief Compiles and links the given vertex and fragment shader codes into an OpenGL program.
     * \param vertexCode The source code for the vertex shader.
     * \param fragmentCode The source code for the fragment shader.
     */
    void compileShader(const std::string& vertexCode, const std::string& fragmentCode);
};
}  // namespace of::vis
