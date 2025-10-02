#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>

namespace Rendering {

// 前向聲明
class Shader;
class Camera;
class ContactVisualizer;

namespace Physics {
    class Particle;
    struct OGCContact;
}

/**
 * @brief OpenGL 渲染器
 * 
 * 負責渲染布料、圓柱體、地板和接觸可視化。
 * 不使用 Qt，直接使用 GLFW 和 OpenGL。
 */
class OpenGLRenderer {
public:
    OpenGLRenderer();
    ~OpenGLRenderer();

    /**
     * @brief 初始化渲染器
     * @param width 視窗寬度
     * @param height 視窗高度
     * @param title 視窗標題
     * @return 是否初始化成功
     */
    bool initialize(int width, int height, const char* title);

    /**
     * @brief 清理資源
     */
    void cleanup();

    /**
     * @brief 開始新的一幀
     */
    void beginFrame();

    /**
     * @brief 結束當前幀
     */
    void endFrame();

    /**
     * @brief 檢查視窗是否應該關閉
     * @return 是否應該關閉
     */
    bool shouldClose();

    /**
     * @brief 處理輸入事件
     */
    void processInput();

    /**
     * @brief 渲染布料粒子
     * @param particles 粒子列表
     */
    void renderClothParticles(const std::vector<Physics::Particle*>& particles);

    /**
     * @brief 渲染布料約束 (彈簧)
     * @param particles 粒子列表
     * @param constraints 約束索引對列表
     */
    void renderClothConstraints(const std::vector<Physics::Particle*>& particles,
                               const std::vector<std::pair<int, int>>& constraints);

    /**
     * @brief 渲染圓柱體
     * @param center 圓柱體中心
     * @param radius 半徑
     * @param height 高度
     * @param color 顏色
     */
    void renderCylinder(const glm::vec3& center, float radius, float height, 
                       const glm::vec3& color = glm::vec3(0.8f, 0.3f, 0.3f));

    /**
     * @brief 渲染地板
     * @param center 地板中心
     * @param size 地板大小
     * @param color 顏色
     */
    void renderFloor(const glm::vec3& center, const glm::vec3& size,
                    const glm::vec3& color = glm::vec3(0.5f, 0.5f, 0.5f));

    /**
     * @brief 渲染接觸點和接觸力
     * @param contacts 接觸列表
     */
    void renderContacts(const std::vector<Physics::OGCContact>& contacts);

    /**
     * @brief 設定相機
     * @param camera 相機指標
     */
    void setCamera(std::shared_ptr<Camera> camera) { m_camera = camera; }

    /**
     * @brief 獲取相機
     * @return 相機指標
     */
    std::shared_ptr<Camera> getCamera() { return m_camera; }

    /**
     * @brief 獲取視窗
     * @return GLFW視窗指標
     */
    GLFWwindow* getWindow() { return m_window; }

    /**
     * @brief 設定視窗大小
     * @param width 寬度
     * @param height 高度
     */
    void setWindowSize(int width, int height);

private:
    GLFWwindow* m_window;
    std::shared_ptr<Shader> m_basicShader;
    std::shared_ptr<Shader> m_lineShader;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<ContactVisualizer> m_contactVisualizer;
    
    int m_windowWidth;
    int m_windowHeight;
    
    // OpenGL 物件
    GLuint m_sphereVAO, m_sphereVBO, m_sphereEBO;
    GLuint m_cylinderVAO, m_cylinderVBO, m_cylinderEBO;
    GLuint m_floorVAO, m_floorVBO, m_floorEBO;
    GLuint m_lineVAO, m_lineVBO;
    
    // 幾何數據
    std::vector<float> m_sphereVertices;
    std::vector<unsigned int> m_sphereIndices;
    std::vector<float> m_cylinderVertices;
    std::vector<unsigned int> m_cylinderIndices;
    std::vector<float> m_floorVertices;
    std::vector<unsigned int> m_floorIndices;
    
    /**
     * @brief 初始化 OpenGL 設定
     */
    void initializeOpenGL();

    /**
     * @brief 創建著色器
     */
    void createShaders();

    /**
     * @brief 創建幾何體
     */
    void createGeometry();

    /**
     * @brief 創建球體幾何
     * @param radius 半徑
     * @param segments 分段數
     */
    void createSphere(float radius = 1.0f, int segments = 16);

    /**
     * @brief 創建圓柱體幾何
     * @param radius 半徑
     * @param height 高度
     * @param segments 分段數
     */
    void createCylinder(float radius = 1.0f, float height = 2.0f, int segments = 16);

    /**
     * @brief 創建地板幾何
     */
    void createFloor();

    /**
     * @brief GLFW 錯誤回調
     */
    static void glfwErrorCallback(int error, const char* description);

    /**
     * @brief 視窗大小改變回調
     */
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

} // namespace Rendering
