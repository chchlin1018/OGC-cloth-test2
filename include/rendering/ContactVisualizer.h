#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace Rendering {

// 前向聲明
class Shader;

namespace Physics {
    struct OGCContact;
}

/**
 * @brief 接觸點和接觸力可視化器
 * 
 * 負責渲染接觸點、接觸法線和接觸力向量。
 */
class ContactVisualizer {
public:
    ContactVisualizer();
    ~ContactVisualizer();

    /**
     * @brief 初始化可視化器
     * @return 是否初始化成功
     */
    bool initialize();

    /**
     * @brief 清理資源
     */
    void cleanup();

    /**
     * @brief 渲染接觸點
     * @param contacts 接觸列表
     * @param viewMatrix 視圖矩陣
     * @param projectionMatrix 投影矩陣
     */
    void renderContacts(const std::vector<Physics::OGCContact>& contacts,
                       const glm::mat4& viewMatrix,
                       const glm::mat4& projectionMatrix);

    /**
     * @brief 渲染接觸點 (球體)
     * @param contacts 接觸列表
     * @param viewMatrix 視圖矩陣
     * @param projectionMatrix 投影矩陣
     * @param pointSize 點大小
     * @param color 顏色
     */
    void renderContactPoints(const std::vector<Physics::OGCContact>& contacts,
                            const glm::mat4& viewMatrix,
                            const glm::mat4& projectionMatrix,
                            float pointSize = 0.05f,
                            const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f));

    /**
     * @brief 渲染接觸法線
     * @param contacts 接觸列表
     * @param viewMatrix 視圖矩陣
     * @param projectionMatrix 投影矩陣
     * @param length 法線長度
     * @param color 顏色
     */
    void renderContactNormals(const std::vector<Physics::OGCContact>& contacts,
                             const glm::mat4& viewMatrix,
                             const glm::mat4& projectionMatrix,
                             float length = 0.2f,
                             const glm::vec3& color = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
     * @brief 渲染接觸力向量
     * @param contacts 接觸列表
     * @param viewMatrix 視圖矩陣
     * @param projectionMatrix 投影矩陣
     * @param forceScale 力的縮放係數
     * @param color 顏色
     */
    void renderContactForces(const std::vector<Physics::OGCContact>& contacts,
                            const glm::mat4& viewMatrix,
                            const glm::mat4& projectionMatrix,
                            float forceScale = 0.001f,
                            const glm::vec3& color = glm::vec3(1.0f, 1.0f, 0.0f));

    /**
     * @brief 渲染 OGC 偏移幾何
     * @param contacts 接觸列表
     * @param viewMatrix 視圖矩陣
     * @param projectionMatrix 投影矩陣
     * @param color 顏色
     */
    void renderOffsetGeometry(const std::vector<Physics::OGCContact>& contacts,
                             const glm::mat4& viewMatrix,
                             const glm::mat4& projectionMatrix,
                             const glm::vec3& color = glm::vec3(0.0f, 0.0f, 1.0f));

    /**
     * @brief 設定是否顯示接觸點
     * @param show 是否顯示
     */
    void setShowContactPoints(bool show) { m_showContactPoints = show; }

    /**
     * @brief 設定是否顯示接觸法線
     * @param show 是否顯示
     */
    void setShowContactNormals(bool show) { m_showContactNormals = show; }

    /**
     * @brief 設定是否顯示接觸力
     * @param show 是否顯示
     */
    void setShowContactForces(bool show) { m_showContactForces = show; }

    /**
     * @brief 設定是否顯示偏移幾何
     * @param show 是否顯示
     */
    void setShowOffsetGeometry(bool show) { m_showOffsetGeometry = show; }

private:
    std::shared_ptr<Shader> m_pointShader;
    std::shared_ptr<Shader> m_lineShader;
    
    // OpenGL 物件
    GLuint m_pointVAO, m_pointVBO;
    GLuint m_lineVAO, m_lineVBO;
    GLuint m_sphereVAO, m_sphereVBO, m_sphereEBO;
    
    // 球體幾何 (用於接觸點)
    std::vector<float> m_sphereVertices;
    std::vector<unsigned int> m_sphereIndices;
    
    // 顯示選項
    bool m_showContactPoints;
    bool m_showContactNormals;
    bool m_showContactForces;
    bool m_showOffsetGeometry;
    
    /**
     * @brief 創建著色器
     */
    void createShaders();
    
    /**
     * @brief 創建幾何體
     */
    void createGeometry();
    
    /**
     * @brief 創建小球體 (用於接觸點)
     * @param radius 半徑
     * @param segments 分段數
     */
    void createSphere(float radius = 1.0f, int segments = 8);
    
    /**
     * @brief 渲染線段
     * @param start 起點
     * @param end 終點
     * @param viewMatrix 視圖矩陣
     * @param projectionMatrix 投影矩陣
     * @param color 顏色
     */
    void renderLine(const glm::vec3& start, const glm::vec3& end,
                   const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                   const glm::vec3& color);
    
    /**
     * @brief 渲染球體
     * @param position 位置
     * @param scale 縮放
     * @param viewMatrix 視圖矩陣
     * @param projectionMatrix 投影矩陣
     * @param color 顏色
     */
    void renderSphere(const glm::vec3& position, float scale,
                     const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                     const glm::vec3& color);
};

} // namespace Rendering
