#include "physics/BulletIntegration.h"
#include "physics/Particle.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#ifdef USE_SIMPLIFIED_COLLISION
// 簡化的碰撞檢測實現，不依賴 Bullet Physics
namespace Physics {

// 簡化的碰撞體結構
struct SimpleCollisionObject {
    enum Type { CYLINDER, BOX, SPHERE };
    Type type;
    glm::vec3 center;
    glm::vec3 size; // 對於圓柱體：x=radius, y=height, z=radius
    Particle* particle; // 如果是粒子，則指向粒子對象
    
    SimpleCollisionObject(Type t, const glm::vec3& c, const glm::vec3& s, Particle* p = nullptr)
        : type(t), center(c), size(s), particle(p) {}
};

class SimpleBulletIntegration {
private:
    std::vector<std::unique_ptr<SimpleCollisionObject>> m_collisionObjects;
    
public:
    SimpleBulletIntegration() {
        std::cout << "Using simplified collision detection (Bullet Physics not available)" << std::endl;
    }
    
    ~SimpleBulletIntegration() {
        m_collisionObjects.clear();
    }
    
    void* addCylinder(const glm::vec3& center, float radius, float height) {
        auto obj = std::make_unique<SimpleCollisionObject>(
            SimpleCollisionObject::CYLINDER, center, glm::vec3(radius, height, radius)
        );
        void* ptr = obj.get();
        m_collisionObjects.push_back(std::move(obj));
        
        std::cout << "Added simplified cylinder: center(" << center.x << ", " << center.y << ", " << center.z 
                  << "), radius=" << radius << ", height=" << height << std::endl;
        return ptr;
    }
    
    void* addFloor(const glm::vec3& center, const glm::vec3& size) {
        auto obj = std::make_unique<SimpleCollisionObject>(
            SimpleCollisionObject::BOX, center, size
        );
        void* ptr = obj.get();
        m_collisionObjects.push_back(std::move(obj));
        
        std::cout << "Added simplified floor: center(" << center.x << ", " << center.y << ", " << center.z 
                  << "), size(" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
        return ptr;
    }
    
    void* addParticle(Particle* particle, float radius) {
        if (!particle) return nullptr;
        
        auto obj = std::make_unique<SimpleCollisionObject>(
            SimpleCollisionObject::SPHERE, particle->getPosition(), glm::vec3(radius), particle
        );
        void* ptr = obj.get();
        m_collisionObjects.push_back(std::move(obj));
        return ptr;
    }
    
    void updateParticlePosition(Particle* particle, void* collisionObject) {
        if (!particle || !collisionObject) return;
        
        SimpleCollisionObject* obj = static_cast<SimpleCollisionObject*>(collisionObject);
        if (obj->particle == particle) {
            obj->center = particle->getPosition();
        }
    }
    
    std::vector<OGCContact> performCollisionDetection() {
        std::vector<OGCContact> contacts;
        
        // 檢查每個粒子與靜態物體的碰撞
        for (const auto& particleObj : m_collisionObjects) {
            if (particleObj->type != SimpleCollisionObject::SPHERE || !particleObj->particle) continue;
            
            for (const auto& staticObj : m_collisionObjects) {
                if (staticObj->particle != nullptr) continue; // 跳過其他粒子
                
                OGCContact contact;
                if (checkCollision(*particleObj, *staticObj, contact)) {
                    contacts.push_back(contact);
                }
            }
        }
        
        return contacts;
    }
    
private:
    bool checkCollision(const SimpleCollisionObject& sphere, const SimpleCollisionObject& other, OGCContact& contact) {
        if (other.type == SimpleCollisionObject::CYLINDER) {
            return checkSphereCylinderCollision(sphere, other, contact);
        } else if (other.type == SimpleCollisionObject::BOX) {
            return checkSphereBoxCollision(sphere, other, contact);
        }
        return false;
    }
    
    bool checkSphereCylinderCollision(const SimpleCollisionObject& sphere, const SimpleCollisionObject& cylinder, OGCContact& contact) {
        glm::vec3 spherePos = sphere.center;
        float sphereRadius = sphere.size.x;
        
        glm::vec3 cylinderPos = cylinder.center;
        float cylinderRadius = cylinder.size.x;
        float cylinderHeight = cylinder.size.y;
        
        // 檢查垂直範圍
        float yMin = cylinderPos.y - cylinderHeight * 0.5f;
        float yMax = cylinderPos.y + cylinderHeight * 0.5f;
        
        if (spherePos.y < yMin - sphereRadius || spherePos.y > yMax + sphereRadius) {
            return false; // 不在圓柱體的高度範圍內
        }
        
        // 計算到圓柱體軸的距離
        glm::vec2 sphereXZ(spherePos.x, spherePos.z);
        glm::vec2 cylinderXZ(cylinderPos.x, cylinderPos.z);
        float distanceToAxis = glm::length(sphereXZ - cylinderXZ);
        
        // 檢查是否碰撞
        float totalRadius = sphereRadius + cylinderRadius;
        if (distanceToAxis < totalRadius) {
            // 計算接觸點和法線
            glm::vec2 direction = glm::normalize(sphereXZ - cylinderXZ);
            glm::vec3 contactNormal = glm::vec3(direction.x, 0.0f, direction.y);
            
            // 限制 Y 座標在圓柱體範圍內
            float contactY = std::max(yMin, std::min(yMax, spherePos.y));
            
            contact.particleA = sphere.particle;
            contact.particleB = nullptr;
            contact.contactPoint = cylinderPos + glm::vec3(direction.x * cylinderRadius, contactY - cylinderPos.y, direction.y * cylinderRadius);
            contact.contactNormal = contactNormal;
            contact.penetrationDepth = totalRadius - distanceToAxis;
            
            return true;
        }
        
        return false;
    }
    
    bool checkSphereBoxCollision(const SimpleCollisionObject& sphere, const SimpleCollisionObject& box, OGCContact& contact) {
        glm::vec3 spherePos = sphere.center;
        float sphereRadius = sphere.size.x;
        
        glm::vec3 boxPos = box.center;
        glm::vec3 boxSize = box.size;
        
        // 計算最近點
        glm::vec3 closestPoint = glm::clamp(spherePos, boxPos - boxSize * 0.5f, boxPos + boxSize * 0.5f);
        
        // 檢查距離
        glm::vec3 diff = spherePos - closestPoint;
        float distance = glm::length(diff);
        
        if (distance < sphereRadius) {
            contact.particleA = sphere.particle;
            contact.particleB = nullptr;
            contact.contactPoint = closestPoint;
            contact.contactNormal = (distance > 0.001f) ? glm::normalize(diff) : glm::vec3(0.0f, 1.0f, 0.0f);
            contact.penetrationDepth = sphereRadius - distance;
            
            return true;
        }
        
        return false;
    }
};

// 全局簡化碰撞檢測實例
static SimpleBulletIntegration g_simpleBullet;

BulletIntegration::BulletIntegration() {
    // 使用簡化實現
}

BulletIntegration::~BulletIntegration() {
    // 簡化實現的清理
}

void BulletIntegration::initialize() {
    // 簡化實現不需要初始化
}

void BulletIntegration::cleanup() {
    // 簡化實現不需要清理
}

btCollisionObject* BulletIntegration::addCylinder(const glm::vec3& center, float radius, float height) {
    return static_cast<btCollisionObject*>(g_simpleBullet.addCylinder(center, radius, height));
}

btCollisionObject* BulletIntegration::addFloor(const glm::vec3& center, const glm::vec3& size) {
    return static_cast<btCollisionObject*>(g_simpleBullet.addFloor(center, size));
}

btCollisionObject* BulletIntegration::addParticle(Particle* particle, float radius) {
    return static_cast<btCollisionObject*>(g_simpleBullet.addParticle(particle, radius));
}

void BulletIntegration::updateParticlePosition(Particle* particle, btCollisionObject* collisionObject) {
    g_simpleBullet.updateParticlePosition(particle, static_cast<void*>(collisionObject));
}

std::vector<OGCContact> BulletIntegration::performCollisionDetection() {
    return g_simpleBullet.performCollisionDetection();
}

void BulletIntegration::removeCollisionObject(btCollisionObject* collisionObject) {
    // 簡化實現暫不支持移除
}

std::vector<OGCContact> BulletIntegration::convertBulletContacts(btPersistentManifold* manifold) {
    // 簡化實現不需要轉換
    return std::vector<OGCContact>();
}

Particle* BulletIntegration::getParticleFromCollisionObject(btCollisionObject* collisionObject) {
    // 簡化實現
    return nullptr;
}

glm::vec3 BulletIntegration::bulletToGlm(const btVector3& btVec) {
    // 簡化實現
    return glm::vec3(0.0f);
}

btVector3 BulletIntegration::glmToBullet(const glm::vec3& glmVec) {
    // 簡化實現
    return btVector3(0, 0, 0);
}

} // namespace Physics

#else
// 原始的 Bullet Physics 實現
#include <btBulletDynamicsCommon.h>

namespace Physics {

BulletIntegration::BulletIntegration() {
    initialize();
}

BulletIntegration::~BulletIntegration() {
    cleanup();
}

void BulletIntegration::initialize() {
    // 創建碰撞配置
    m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    
    // 創建碰撞調度器
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());
    
    // 創建廣相碰撞檢測
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    
    // 創建碰撞世界 (只用於碰撞檢測，不進行物理模擬)
    m_collisionWorld = std::make_unique<btCollisionWorld>(
        m_dispatcher.get(), 
        m_broadphase.get(), 
        m_collisionConfig.get()
    );
    
    std::cout << "Bullet Physics collision detection initialized" << std::endl;
}

void BulletIntegration::cleanup() {
    // 清理碰撞物件
    for (auto& obj : m_collisionObjects) {
        if (m_collisionWorld) {
            m_collisionWorld->removeCollisionObject(obj.get());
        }
    }
    m_collisionObjects.clear();
    
    // 清理碰撞形狀
    m_collisionShapes.clear();
    
    // 清理 Bullet 世界
    m_collisionWorld.reset();
    m_broadphase.reset();
    m_dispatcher.reset();
    m_collisionConfig.reset();
    
    std::cout << "Bullet Physics cleaned up" << std::endl;
}

btCollisionObject* BulletIntegration::addCylinder(const glm::vec3& center, float radius, float height) {
    // 創建圓柱體碰撞形狀
    auto cylinderShape = std::make_unique<btCylinderShape>(btVector3(radius, height * 0.5f, radius));
    btCollisionShape* shapePtr = cylinderShape.get();
    m_collisionShapes.push_back(std::move(cylinderShape));
    
    // 創建碰撞物件
    auto collisionObject = std::make_unique<btCollisionObject>();
    collisionObject->setCollisionShape(shapePtr);
    
    // 設定位置
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(glmToBullet(center));
    collisionObject->setWorldTransform(transform);
    
    // 設定用戶指標為nullptr (表示靜態物體)
    collisionObject->setUserPointer(nullptr);
    
    // 添加到碰撞世界
    btCollisionObject* objPtr = collisionObject.get();
    m_collisionWorld->addCollisionObject(objPtr);
    m_collisionObjects.push_back(std::move(collisionObject));
    
    std::cout << "Added cylinder: center(" << center.x << ", " << center.y << ", " << center.z 
              << "), radius=" << radius << ", height=" << height << std::endl;
    
    return objPtr;
}

btCollisionObject* BulletIntegration::addFloor(const glm::vec3& center, const glm::vec3& size) {
    // 創建盒子碰撞形狀作為地板
    auto boxShape = std::make_unique<btBoxShape>(btVector3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));
    btCollisionShape* shapePtr = boxShape.get();
    m_collisionShapes.push_back(std::move(boxShape));
    
    // 創建碰撞物件
    auto collisionObject = std::make_unique<btCollisionObject>();
    collisionObject->setCollisionShape(shapePtr);
    
    // 設定位置
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(glmToBullet(center));
    collisionObject->setWorldTransform(transform);
    
    // 設定用戶指標為nullptr (表示靜態物體)
    collisionObject->setUserPointer(nullptr);
    
    // 添加到碰撞世界
    btCollisionObject* objPtr = collisionObject.get();
    m_collisionWorld->addCollisionObject(objPtr);
    m_collisionObjects.push_back(std::move(collisionObject));
    
    std::cout << "Added floor: center(" << center.x << ", " << center.y << ", " << center.z 
              << "), size(" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
    
    return objPtr;
}

btCollisionObject* BulletIntegration::addParticle(Particle* particle, float radius) {
    if (!particle) return nullptr;
    
    // 創建球體碰撞形狀
    auto sphereShape = std::make_unique<btSphereShape>(radius);
    btCollisionShape* shapePtr = sphereShape.get();
    m_collisionShapes.push_back(std::move(sphereShape));
    
    // 創建碰撞物件
    auto collisionObject = std::make_unique<btCollisionObject>();
    collisionObject->setCollisionShape(shapePtr);
    
    // 設定初始位置
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(glmToBullet(particle->getPosition()));
    collisionObject->setWorldTransform(transform);
    
    // 設定用戶指標指向粒子
    collisionObject->setUserPointer(particle);
    
    // 添加到碰撞世界
    btCollisionObject* objPtr = collisionObject.get();
    m_collisionWorld->addCollisionObject(objPtr);
    m_collisionObjects.push_back(std::move(collisionObject));
    
    return objPtr;
}

void BulletIntegration::updateParticlePosition(Particle* particle, btCollisionObject* collisionObject) {
    if (!particle || !collisionObject) return;
    
    // 更新碰撞物件的位置
    btTransform transform = collisionObject->getWorldTransform();
    transform.setOrigin(glmToBullet(particle->getPosition()));
    collisionObject->setWorldTransform(transform);
}

std::vector<OGCContact> BulletIntegration::performCollisionDetection() {
    std::vector<OGCContact> contacts;
    
    // 執行碰撞檢測
    m_collisionWorld->performDiscreteCollisionDetection();
    
    // 遍歷所有接觸流形
    int numManifolds = m_dispatcher->getNumManifolds();
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
        
        // 轉換 Bullet 接觸為 OGC 接觸
        std::vector<OGCContact> manifoldContacts = convertBulletContacts(manifold);
        contacts.insert(contacts.end(), manifoldContacts.begin(), manifoldContacts.end());
    }
    
    return contacts;
}

void BulletIntegration::removeCollisionObject(btCollisionObject* collisionObject) {
    if (!collisionObject) return;
    
    // 從碰撞世界移除
    m_collisionWorld->removeCollisionObject(collisionObject);
    
    // 從容器中移除
    m_collisionObjects.erase(
        std::remove_if(m_collisionObjects.begin(), m_collisionObjects.end(),
            [collisionObject](const std::unique_ptr<btCollisionObject>& obj) {
                return obj.get() == collisionObject;
            }),
        m_collisionObjects.end()
    );
}

std::vector<OGCContact> BulletIntegration::convertBulletContacts(btPersistentManifold* manifold) {
    std::vector<OGCContact> contacts;
    
    if (!manifold) return contacts;
    
    const btCollisionObject* objA = manifold->getBody0();
    const btCollisionObject* objB = manifold->getBody1();
    
    Particle* particleA = getParticleFromCollisionObject(const_cast<btCollisionObject*>(objA));
    Particle* particleB = getParticleFromCollisionObject(const_cast<btCollisionObject*>(objB));
    
    // 遍歷接觸點
    int numContacts = manifold->getNumContacts();
    for (int i = 0; i < numContacts; i++) {
        btManifoldPoint& point = manifold->getContactPoint(i);
        
        // 只處理有效的接觸點
        if (point.getDistance() < 0.1f) { // 接觸閾值
            OGCContact contact;
            
            contact.particleA = particleA;
            contact.particleB = particleB;
            contact.contactPoint = bulletToGlm(point.getPositionWorldOnA());
            contact.contactNormal = bulletToGlm(point.m_normalWorldOnB);
            contact.penetrationDepth = std::max(0.0f, -point.getDistance());
            
            contacts.push_back(contact);
        }
    }
    
    return contacts;
}

Particle* BulletIntegration::getParticleFromCollisionObject(btCollisionObject* collisionObject) {
    if (!collisionObject) return nullptr;
    return static_cast<Particle*>(collisionObject->getUserPointer());
}

glm::vec3 BulletIntegration::bulletToGlm(const btVector3& btVec) {
    return glm::vec3(btVec.x(), btVec.y(), btVec.z());
}

btVector3 BulletIntegration::glmToBullet(const glm::vec3& glmVec) {
    return btVector3(glmVec.x, glmVec.y, glmVec.z);
}

} // namespace Physics

#endif // USE_SIMPLIFIED_COLLISION
