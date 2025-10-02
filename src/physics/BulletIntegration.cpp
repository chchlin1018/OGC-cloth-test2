#include "physics/BulletIntegration.h"
#include "physics/Particle.h"
#include <iostream>

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
