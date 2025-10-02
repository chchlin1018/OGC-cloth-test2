#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Rendering {

/**
 * @brief 3D 相機類
 * 
 * 提供視圖和投影矩陣，支援滑鼠和鍵盤控制。
 */
class Camera {
public:
    /**
     * @brief 構造函數
     * @param position 相機位置
     * @param target 目標位置
     * @param up 上方向
     */
    Camera(const glm::vec3& position = glm::vec3(0.0f, 5.0f, 10.0f),
           const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    ~Camera() = default;

    /**
     * @brief 獲取視圖矩陣
     * @return 視圖矩陣
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief 獲取投影矩陣
     * @param aspect 寬高比
     * @param fov 視野角度 (度)
     * @param nearPlane 近平面
     * @param farPlane 遠平面
     * @return 投影矩陣
     */
    glm::mat4 getProjectionMatrix(float aspect, float fov = 45.0f, 
                                 float nearPlane = 0.1f, float farPlane = 100.0f) const;

    /**
     * @brief 處理滑鼠移動 (軌道相機)
     * @param xOffset X軸偏移
     * @param yOffset Y軸偏移
     * @param sensitivity 靈敏度
     */
    void processMouseMovement(float xOffset, float yOffset, float sensitivity = 0.01f);

    /**
     * @brief 處理滑鼠滾輪 (縮放)
     * @param yOffset 滾輪偏移
     * @param sensitivity 靈敏度
     */
    void processMouseScroll(float yOffset, float sensitivity = 1.0f);

    /**
     * @brief 處理鍵盤輸入
     * @param deltaTime 時間增量
     */
    void processKeyboard(int key, float deltaTime);

    /**
     * @brief 重置相機
     */
    void reset();

    /**
     * @brief 設定目標位置
     * @param target 新的目標位置
     */
    void setTarget(const glm::vec3& target) { m_target = target; updateCameraVectors(); }

    /**
     * @brief 獲取相機位置
     * @return 相機位置
     */
    const glm::vec3& getPosition() const { return m_position; }

    /**
     * @brief 獲取目標位置
     * @return 目標位置
     */
    const glm::vec3& getTarget() const { return m_target; }

private:
    // 相機屬性
    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_front;
    
    // 初始值 (用於重置)
    glm::vec3 m_initialPosition;
    glm::vec3 m_initialTarget;
    glm::vec3 m_initialUp;
    
    // 軌道相機參數
    float m_distance;           // 到目標的距離
    float m_azimuth;           // 方位角 (水平旋轉)
    float m_elevation;         // 仰角 (垂直旋轉)
    
    // 約束
    float m_minDistance;
    float m_maxDistance;
    float m_minElevation;
    float m_maxElevation;
    
    /**
     * @brief 更新相機向量
     */
    void updateCameraVectors();
    
    /**
     * @brief 從球座標更新位置
     */
    void updatePositionFromSpherical();
};

} // namespace Rendering
