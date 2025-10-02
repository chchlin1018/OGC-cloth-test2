#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "physics/Particle.h"
#include "physics/OGCContactModel.h"

namespace Physics {

// 前向聲明
class BulletIntegration;

/**
 * @brief 布料約束結構
 */
struct ClothConstraint {
    int particleA;          // 粒子A索引
    int particleB;          // 粒子B索引
    float restLength;       // 靜止長度
    float stiffness;        // 剛度
    float damping;          // 阻尼
    
    ClothConstraint(int a, int b, float length, float k = 1000.0f, float d = 10.0f)
        : particleA(a), particleB(b), restLength(length), stiffness(k), damping(d) {}
};

/**
 * @brief 布料模擬類
 * 
 * 實現基於粒子的布料物理模擬，使用 Verlet 積分和約束求解。
 * 整合 OGC Contact Model 和 Bullet Physics 進行碰撞檢測。
 */
class ClothSimulation {
public:
    ClothSimulation();
    ~ClothSimulation();

    /**
     * @brief 初始化布料模擬
     * @param width 布料寬度 (粒子數)
     * @param height 布料高度 (粒子數)
     * @param clothSize 布料實際大小
     * @param position 布料初始位置
     * @param particleMass 粒子質量
     * @return 是否初始化成功
     */
    bool initialize(int width, int height, 
                   const glm::vec2& clothSize = glm::vec2(2.0f, 2.0f),
                   const glm::vec3& position = glm::vec3(0.0f, 3.0f, 0.0f),
                   float particleMass = 0.1f);

    /**
     * @brief 清理資源
     */
    void cleanup();

    /**
     * @brief 更新模擬
     * @param deltaTime 時間步長
     */
    void update(float deltaTime);

    /**
     * @brief 添加圓柱體碰撞體
     * @param center 圓柱體中心
     * @param radius 半徑
     * @param height 高度
     */
    void addCylinder(const glm::vec3& center, float radius, float height);

    /**
     * @brief 添加地板碰撞體
     * @param center 地板中心
     * @param size 地板大小
     */
    void addFloor(const glm::vec3& center, const glm::vec3& size);

    /**
     * @brief 設定重力
     * @param gravity 重力向量
     */
    void setGravity(const glm::vec3& gravity) { m_gravity = gravity; }

    /**
     * @brief 設定風力
     * @param wind 風力向量
     */
    void setWind(const glm::vec3& wind) { m_wind = wind; }

    /**
     * @brief 設定阻尼係數
     * @param damping 阻尼係數
     */
    void setDamping(float damping) { m_damping = damping; }

    /**
     * @brief 固定粒子 (釘住布料的某些點)
     * @param particleIndex 粒子索引
     * @param fixed 是否固定
     */
    void setParticleFixed(int particleIndex, bool fixed);

    /**
     * @brief 獲取粒子列表
     * @return 粒子指標列表
     */
    const std::vector<std::unique_ptr<Particle>>& getParticles() const { return m_particles; }

    /**
     * @brief 獲取約束列表
     * @return 約束列表
     */
    const std::vector<ClothConstraint>& getConstraints() const { return m_constraints; }

    /**
     * @brief 獲取當前接觸列表
     * @return OGC接觸列表
     */
    const std::vector<OGCContact>& getContacts() const { return m_contacts; }

    /**
     * @brief 獲取布料尺寸
     * @return 寬度和高度 (粒子數)
     */
    std::pair<int, int> getClothSize() const { return {m_width, m_height}; }

    /**
     * @brief 重置布料到初始狀態
     */
    void reset();

private:
    // 布料參數
    int m_width, m_height;
    glm::vec2 m_clothSize;
    glm::vec3 m_initialPosition;
    float m_particleMass;
    
    // 物理參數
    glm::vec3 m_gravity;
    glm::vec3 m_wind;
    float m_damping;
    
    // 約束參數
    float m_structuralStiffness;    // 結構約束剛度
    float m_shearStiffness;         // 剪切約束剛度
    float m_bendingStiffness;       // 彎曲約束剛度
    int m_constraintIterations;     // 約束迭代次數
    
    // 模擬數據
    std::vector<std::unique_ptr<Particle>> m_particles;
    std::vector<ClothConstraint> m_constraints;
    std::vector<OGCContact> m_contacts;
    
    // 碰撞檢測和接觸模型
    std::unique_ptr<BulletIntegration> m_bulletIntegration;
    std::unique_ptr<OGCContactModel> m_ogcContactModel;
    
    /**
     * @brief 創建粒子網格
     */
    void createParticles();
    
    /**
     * @brief 創建約束
     */
    void createConstraints();
    
    /**
     * @brief 應用外力 (重力、風力等)
     * @param deltaTime 時間步長
     */
    void applyForces(float deltaTime);
    
    /**
     * @brief 更新粒子位置 (Verlet 積分)
     * @param deltaTime 時間步長
     */
    void updateParticles(float deltaTime);
    
    /**
     * @brief 求解約束
     */
    void solveConstraints();
    
    /**
     * @brief 處理碰撞
     */
    void handleCollisions();
    
    /**
     * @brief 計算風力
     * @param p1 粒子1
     * @param p2 粒子2
     * @param p3 粒子3
     * @return 風力向量
     */
    glm::vec3 calculateWindForce(const Particle& p1, const Particle& p2, const Particle& p3);
    
    /**
     * @brief 獲取粒子索引
     * @param x X座標
     * @param y Y座標
     * @return 粒子索引
     */
    int getParticleIndex(int x, int y) const { return y * m_width + x; }
    
    /**
     * @brief 檢查索引是否有效
     * @param x X座標
     * @param y Y座標
     * @return 是否有效
     */
    bool isValidIndex(int x, int y) const { 
        return x >= 0 && x < m_width && y >= 0 && y < m_height; 
    }
};

} // namespace Physics
