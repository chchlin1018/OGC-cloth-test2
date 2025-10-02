#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

// 前向聲明
namespace Physics {
    struct OGCContact;
}

namespace Rendering {

// 前向聲明
class Shader;

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
     */
    void initialize();

    /**
     * @brief 清理資源
     */
    void cleanup();

    /**
     * @brief 渲染接觸點
     * @param contacts 接觸列表
     * @param view 視圖矩陣
     * @param projection 投影矩陣
     */
    void renderContactPoints(const std::vector<::Physics::OGCContact>& contacts,
                           const glm::mat4& view, const glm::mat4& projection);

    /**
     * @brief 渲染接觸法線
     * @param contacts 接觸列表
     * @param view 視圖矩陣
     * @param projection 投影矩陣
     */
    void renderContactNormals(const std::vector<::Physics::OGCContact>& contacts,
                            const glm::mat4& view, const glm::mat4& projection);

    /**
     * @brief 渲染接觸力向量
     * @param contacts 接觸列表
     * @param view 視圖矩陣
     * @param projection 投影矩陣
     */
    void renderContactForces(const std::vector<::Physics::OGCContact>& contacts,
                           const glm::mat4& view, const glm::mat4& projection);

    /**
     * @brief 渲染 OGC 偏移幾何
     * @param contacts 接觸列表
     * @param view 視圖矩陣
     * @param projection 投影矩陣
     */
    void renderOffsetGeometry(const std::vector<::Physics::OGCContact>& contacts,
                            const glm::mat4& view, const glm::mat4& projection);

private:
    // OpenGL 對象
    GLuint m_pointVAO, m_pointVBO;
    GLuint m_lineVAO, m_lineVBO;
    GLuint m_sphereVAO, m_sphereVBO, m_sphereEBO;
    
    // 著色器
    std::unique_ptr<Shader> m_pointShader;
    std::unique_ptr<Shader> m_lineShader;
    std::unique_ptr<Shader> m_sphereShader;
    
    // 球體幾何數據
    std::vector<float> m_sphereVertices;
    std::vector<unsigned int> m_sphereIndices;
    
    /**
     * @brief 初始化著色器
     */
    void initializeShaders();
    
    /**
     * @brief 初始化幾何數據
     */
    void initializeGeometry();
    
    /**
     * @brief 生成球體幾何數據
     * @param radius 球體半徑
     * @param sectorCount 經度分割數
     * @param stackCount 緯度分割數
     */
    void generateSphere(float radius = 1.0f, int sectorCount = 12, int stackCount = 6);
    
    /**
     * @brief 渲染線段
     * @param start 起始點
     * @param end 結束點
     * @param view 視圖矩陣
     * @param projection 投影矩陣
     * @param color 顏色
     */
    void renderLine(const glm::vec3& start, const glm::vec3& end,
                   const glm::mat4& view, const glm::mat4& projection,
                   const glm::vec3& color = glm::vec3(1.0f));
    
    /**
     * @brief 渲染球體
     * @param position 位置
     * @param scale 縮放
     * @param view 視圖矩陣
     * @param projection 投影矩陣
     * @param color 顏色
     */
    void renderSphere(const glm::vec3& position, float scale,
                     const glm::mat4& view, const glm::mat4& projection,
                     const glm::vec3& color = glm::vec3(1.0f));
};

} // namespace Rendering
