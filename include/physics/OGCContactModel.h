#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Physics {

// 前向聲明
class Particle;

/**
 * @brief OGC 接觸信息結構
 */
struct OGCContact {
    Particle* particleA;           // 接觸粒子A
    Particle* particleB;           // 接觸粒子B (可能為nullptr，表示與靜態物體接觸)
    glm::vec3 contactPoint;        // 接觸點位置
    glm::vec3 contactNormal;       // 接觸法線 (從A指向B)
    float penetrationDepth;        // 穿透深度
    float contactRadius;           // OGC接觸半徑
    glm::vec3 offsetGeometry;      // OGC偏移幾何
    float contactForce;            // 接觸力大小
    glm::vec3 forceDirection;      // 接觸力方向
    
    OGCContact() : particleA(nullptr), particleB(nullptr), 
                   contactPoint(0.0f), contactNormal(0.0f, 1.0f, 0.0f),
                   penetrationDepth(0.0f), contactRadius(0.05f),
                   offsetGeometry(0.0f), contactForce(0.0f),
                   forceDirection(0.0f) {}
};

/**
 * @brief OGC (Offset Geometric Contact) 接觸模型
 * 
 * 實現基於偏移幾何的接觸模型，提供更穩定和準確的接觸處理。
 * 主要特點：
 * 1. 使用偏移幾何來預測和處理接觸
 * 2. 保證無穿透模擬
 * 3. 提供正交接觸力
 * 4. 支援軟體接觸和硬體接觸
 */
class OGCContactModel {
public:
    /**
     * @brief 構造函數
     * @param contactRadius 接觸半徑
     * @param stiffness 接觸剛度
     * @param damping 接觸阻尼
     */
    OGCContactModel(float contactRadius = 0.05f, 
                    float stiffness = 1000.0f, 
                    float damping = 50.0f);
    
    ~OGCContactModel() = default;

    /**
     * @brief 處理接觸列表
     * @param contacts 接觸列表
     * @param deltaTime 時間步長
     */
    void processContacts(std::vector<OGCContact>& contacts, float deltaTime);

    /**
     * @brief 計算OGC偏移幾何
     * @param contact 接觸信息
     * @return 偏移幾何向量
     */
    glm::vec3 calculateOffsetGeometry(const OGCContact& contact);

    /**
     * @brief 應用OGC接觸力
     * @param contact 接觸信息
     * @param deltaTime 時間步長
     */
    void applyOGCForce(OGCContact& contact, float deltaTime);

    /**
     * @brief 計算接觸力大小和方向
     * @param contact 接觸信息
     * @param deltaTime 時間步長
     */
    void calculateContactForce(OGCContact& contact, float deltaTime);

    /**
     * @brief 執行位置修正以防止穿透
     * @param contact 接觸信息
     */
    void performPositionCorrection(const OGCContact& contact);

    // Getter 和 Setter
    void setContactRadius(float radius) { m_contactRadius = radius; }
    float getContactRadius() const { return m_contactRadius; }
    
    void setStiffness(float stiffness) { m_stiffness = stiffness; }
    float getStiffness() const { return m_stiffness; }
    
    void setDamping(float damping) { m_damping = damping; }
    float getDamping() const { return m_damping; }
    
    void setPositionCorrectionFactor(float factor) { m_positionCorrectionFactor = factor; }
    float getPositionCorrectionFactor() const { return m_positionCorrectionFactor; }

private:
    float m_contactRadius;              // 接觸半徑
    float m_stiffness;                  // 接觸剛度
    float m_damping;                    // 接觸阻尼
    float m_positionCorrectionFactor;   // 位置修正係數
    
    /**
     * @brief 計算相對速度
     * @param contact 接觸信息
     * @return 相對速度
     */
    glm::vec3 calculateRelativeVelocity(const OGCContact& contact);
    
    /**
     * @brief 計算法線速度
     * @param contact 接觸信息
     * @return 法線方向的相對速度
     */
    float calculateNormalVelocity(const OGCContact& contact);
};

} // namespace Physics
