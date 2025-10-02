#include "rendering/Shader.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace Rendering {

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource)
    : m_program(0)
{
    // 編譯頂點著色器
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) return;
    
    // 編譯片段著色器
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return;
    }
    
    // 連結程序
    m_program = linkProgram(vertexShader, fragmentShader);
    
    // 清理著色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

void Shader::use() {
    if (m_program != 0) {
        glUseProgram(m_program);
    }
}

void Shader::setMatrix4(const std::string& name, const glm::mat4& matrix) {
    if (m_program == 0) return;
    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}

void Shader::setVector3(const std::string& name, const glm::vec3& vector) {
    if (m_program == 0) return;
    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(vector));
    }
}

void Shader::setFloat(const std::string& name, float value) {
    if (m_program == 0) return;
    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Shader::setInt(const std::string& name, int value) {
    if (m_program == 0) return;
    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location != -1) {
        glUniform1i(location, value);
    }
}

GLuint Shader::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    // 檢查編譯錯誤
    std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkCompileErrors(shader, typeStr);
    
    // 檢查編譯狀態
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // 檢查連結錯誤
    checkCompileErrors(program, "PROGRAM");
    
    // 檢查連結狀態
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

} // namespace Rendering
