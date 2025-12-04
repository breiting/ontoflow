#include <fstream>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/vis/Shader.hpp>
#include <sstream>

namespace of::vis {

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    compileShader(vertexSource, fragmentSource);
}

Shader::~Shader() {
    glDeleteProgram(m_ID);
}

std::string Shader::readShaderFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream fileStream;

    if (!file.is_open()) {
        LOG(Error) << "Shader: Failed to open file: " << path;
        return "";
    }

    fileStream << file.rdbuf();
    return fileStream.str();
}

void Shader::compileShader(const std::string& vertexCode, const std::string& fragmentCode) {
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        LOG(Error) << "Shader: Error compiling vertex shader: " << infoLog;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        LOG(Error) << "Shader: Error compiling fragment shader: " << infoLog;
    }

    // Shader-Programm creation
    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertex);
    glAttachShader(m_ID, fragment);
    glLinkProgram(m_ID);
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
        LOG(Error) << "Shader: Error linking shader program: " << infoLog;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::Bind() {
    glUseProgram(m_ID);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& matrix) {
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::SetBool(const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), static_cast<int>(value));
}

int Shader::GetUniformLocation(const std::string& name) const {
    return glGetUniformLocation(m_ID, name.c_str());
}
}  // namespace of::vis
