#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace Rendering {

/**
 * @brief OpenGL 著色器類
 * 
 * 負責載入、編譯和使用 OpenGL 著色器程序。
 */
class Shader {
public:
    /**
     * @brief 構造函數
     * @param vertexSource 頂點著色器源碼
     * @param fragmentSource 片段著色器源碼
     */
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    
    ~Shader();

    /**
     * @brief 使用此著色器程序
     */
    void use();

    /**
     * @brief 獲取著色器程序 ID
     * @return OpenGL 程序 ID
     */
    GLuint getProgram() const { return m_program; }

    /**
     * @brief 設定 uniform 變數
     */
    void setMatrix4(const std::string& name, const glm::mat4& matrix);
    void setVector3(const std::string& name, const glm::vec3& vector);
    void setFloat(const std::string& name, float value);
    void setInt(const std::string& name, int value);
    
    // 別名方法，與常見的OpenGL教程保持一致
    void setMat4(const std::string& name, const glm::mat4& matrix) { setMatrix4(name, matrix); }
    void setVec3(const std::string& name, const glm::vec3& vector) { setVector3(name, vector); }

    /**
     * @brief 檢查著色器是否有效
     * @return 是否有效
     */
    bool isValid() const { return m_program != 0; }

private:
    GLuint m_program;
    
    /**
     * @brief 編譯著色器
     * @param source 著色器源碼
     * @param type 著色器類型
     * @return 著色器 ID
     */
    GLuint compileShader(const std::string& source, GLenum type);
    
    /**
     * @brief 連結著色器程序
     * @param vertexShader 頂點著色器 ID
     * @param fragmentShader 片段著色器 ID
     * @return 程序 ID
     */
    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
    
    /**
     * @brief 檢查編譯錯誤
     * @param shader 著色器 ID
     * @param type 檢查類型
     */
    void checkCompileErrors(GLuint shader, const std::string& type);
};

} // namespace Rendering
