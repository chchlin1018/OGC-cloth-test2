#pragma once

#include <glm/glm.hpp>

namespace Physics {

/**
 * @brief 粒子類
 * 
 * 表示布料模擬中的一個粒子，包含位置、速度、力等物理屬性。
 */
class Particle {
public:
    /**
     * @brief 構造函數
     * @param position 初始位置
     * @param mass 質量
     */
    Particle(const glm::vec3& position = glm::vec3(0.0f), float mass = 1.0f);
    
    ~Particle() = default;

    /**
     * @brief 更新粒子狀態 (Verlet積分)
     * @param deltaTime 時間步長
     */
    void update(float deltaTime);

    /**
     * @brief 添加力
     * @param force 要添加的力
     */
    void addForce(const glm::vec3& force);

    /**
     * @brief 清除所有力
     */
    void clearForces();

    /**
     * @brief 設定位置
     * @param position 新位置
     */
    void setPosition(const glm::vec3& position);

    /**
     * @brief 獲取位置
     * @return 當前位置
     */
    const glm::vec3& getPosition() const { return m_position; }

    /**
     * @brief 獲取上一幀位置
     * @return 上一幀位置
     */
    const glm::vec3& getPreviousPosition() const { return m_previousPosition; }

    /**
     * @brief 獲取速度
     * @return 當前速度
     */
    glm::vec3 getVelocity() const;

    /**
     * @brief 設定速度
     * @param velocity 新速度
     */
    void setVelocity(const glm::vec3& velocity);

    /**
     * @brief 獲取質量
     * @return 質量
     */
    float getMass() const { return m_mass; }

    /**
     * @brief 獲取逆質量
     * @return 逆質量 (1/質量)
     */
    float getInverseMass() const { return m_inverseMass; }

    /**
     * @brief 設定質量
     * @param mass 新質量
     */
    void setMass(float mass);

    /**
     * @brief 檢查是否為固定粒子
     * @return 是否固定
     */
    bool isFixed() const { return m_inverseMass == 0.0f; }

    /**
     * @brief 設定為固定粒子
     * @param fixed 是否固定
     */
    void setFixed(bool fixed);

    /**
     * @brief 獲取累積力
     * @return 當前累積的力
     */
    const glm::vec3& getAccumulatedForce() const { return m_force; }

private:
    glm::vec3 m_position;           // 當前位置
    glm::vec3 m_previousPosition;   // 上一幀位置
    glm::vec3 m_force;              // 累積力
    float m_mass;                   // 質量
    float m_inverseMass;            // 逆質量
};

} // namespace Physics
