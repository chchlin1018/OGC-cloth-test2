#pragma once

#include <btBulletCollisionCommon.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "physics/OGCContactModel.h"

namespace Physics {

// 前向聲明
class Particle;

/**
 * @brief Bullet Physics 整合類
 * 
 * 負責將 Bullet Physics 的碰撞檢測功能整合到 OGC 系統中。
 * Bullet 主要用於高效的碰撞檢測，而 OGC 負責接觸解析。
 */
class BulletIntegration {
public:
    BulletIntegration();
    ~BulletIntegration();

    /**
     * @brief 初始化 Bullet Physics 世界
     */
    void initialize();

    /**
     * @brief 清理資源
     */
    void cleanup();

    /**
     * @brief 添加圓柱體碰撞體
     * @param center 圓柱體中心位置
     * @param radius 圓柱體半徑
     * @param height 圓柱體高度
     * @return 碰撞物件指標
     */
    btCollisionObject* addCylinder(const glm::vec3& center, float radius, float height);

    /**
     * @brief 添加地板碰撞體
     * @param center 地板中心位置
     * @param size 地板大小
     * @return 碰撞物件指標
     */
    btCollisionObject* addFloor(const glm::vec3& center, const glm::vec3& size);

    /**
     * @brief 添加粒子碰撞體
     * @param particle 粒子指標
     * @param radius 粒子半徑
     * @return 碰撞物件指標
     */
    btCollisionObject* addParticle(Particle* particle, float radius);

    /**
     * @brief 更新粒子位置
     * @param particle 粒子指標
     * @param collisionObject 對應的碰撞物件
     */
    void updateParticlePosition(Particle* particle, btCollisionObject* collisionObject);

    /**
     * @brief 執行碰撞檢測
     * @return OGC接觸列表
     */
    std::vector<OGCContact> performCollisionDetection();

    /**
     * @brief 移除碰撞物件
     * @param collisionObject 要移除的碰撞物件
     */
    void removeCollisionObject(btCollisionObject* collisionObject);

    /**
     * @brief 獲取碰撞世界
     * @return Bullet碰撞世界指標
     */
    btCollisionWorld* getCollisionWorld() { return m_collisionWorld.get(); }

private:
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btDbvtBroadphase> m_broadphase;
    std::unique_ptr<btCollisionWorld> m_collisionWorld;
    
    std::vector<std::unique_ptr<btCollisionShape>> m_collisionShapes;
    std::vector<std::unique_ptr<btCollisionObject>> m_collisionObjects;
    
    /**
     * @brief 將 Bullet 接觸點轉換為 OGC 接觸
     * @param manifold Bullet 接觸流形
     * @return OGC接觸列表
     */
    std::vector<OGCContact> convertBulletContacts(btPersistentManifold* manifold);
    
    /**
     * @brief 從碰撞物件獲取粒子指標
     * @param collisionObject 碰撞物件
     * @return 粒子指標 (如果不是粒子則返回nullptr)
     */
    Particle* getParticleFromCollisionObject(btCollisionObject* collisionObject);
    
    /**
     * @brief 將 Bullet 向量轉換為 GLM 向量
     * @param btVec Bullet向量
     * @return GLM向量
     */
    glm::vec3 bulletToGlm(const btVector3& btVec);
    
    /**
     * @brief 將 GLM 向量轉換為 Bullet 向量
     * @param glmVec GLM向量
     * @return Bullet向量
     */
    btVector3 glmToBullet(const glm::vec3& glmVec);
};

} // namespace Physics
