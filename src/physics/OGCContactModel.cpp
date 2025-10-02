#include "physics/OGCContactModel.h"
#include "physics/Particle.h"
#include <algorithm>
#include <cmath>

namespace Physics {

OGCContactModel::OGCContactModel(float contactRadius, float stiffness, float damping)
    : m_contactRadius(contactRadius)
    , m_stiffness(stiffness)
    , m_damping(damping)
    , m_positionCorrectionFactor(0.8f)
{
}

void OGCContactModel::processContacts(std::vector<OGCContact>& contacts, float deltaTime) {
    for (auto& contact : contacts) {
        // 1. 計算OGC偏移幾何
        contact.offsetGeometry = calculateOffsetGeometry(contact);
        
        // 2. 計算接觸力
        calculateContactForce(contact, deltaTime);
        
        // 3. 應用OGC接觸力
        applyOGCForce(contact, deltaTime);
        
        // 4. 執行位置修正
        performPositionCorrection(contact);
    }
}

glm::vec3 OGCContactModel::calculateOffsetGeometry(const OGCContact& contact) {
    // OGC模型的核心：計算偏移幾何
    // 偏移幾何 = 接觸半徑 * 接觸法線
    glm::vec3 offset = m_contactRadius * contact.contactNormal;
    
    // 考慮穿透深度的影響
    if (contact.penetrationDepth > 0.0f) {
        // 當有穿透時，增加偏移以確保分離
        float additionalOffset = contact.penetrationDepth * 0.5f;
        offset += additionalOffset * contact.contactNormal;
    }
    
    return offset;
}

void OGCContactModel::calculateContactForce(OGCContact& contact, float deltaTime) {
    if (!contact.particleA) return;
    
    // 計算相對速度
    glm::vec3 relativeVelocity = calculateRelativeVelocity(contact);
    float normalVelocity = calculateNormalVelocity(contact);
    
    // OGC彈簧力：基於穿透深度和偏移幾何
    float springForce = 0.0f;
    if (contact.penetrationDepth > 0.0f) {
        // 考慮OGC偏移的彈簧力
        float effectivePenetration = contact.penetrationDepth + glm::length(contact.offsetGeometry);
        springForce = m_stiffness * effectivePenetration;
    }
    
    // OGC阻尼力：基於法線速度
    float dampingForce = m_damping * normalVelocity;
    
    // 總接觸力 (只在法線方向)
    contact.contactForce = std::max(0.0f, springForce - dampingForce);
    contact.forceDirection = contact.contactNormal;
    
    // 確保力的方向正確 (從接觸表面推開)
    if (normalVelocity < 0.0f) {
        // 物體正在分離，不施加額外的推力
        contact.contactForce = std::max(0.0f, contact.contactForce);
    }
}

void OGCContactModel::applyOGCForce(OGCContact& contact, float deltaTime) {
    if (!contact.particleA || contact.contactForce <= 0.0f) return;
    
    glm::vec3 force = contact.contactForce * contact.forceDirection;
    
    // 對粒子A施加力
    contact.particleA->addForce(force);
    
    // 如果有粒子B，施加反作用力
    if (contact.particleB) {
        contact.particleB->addForce(-force);
    }
}

void OGCContactModel::performPositionCorrection(const OGCContact& contact) {
    if (!contact.particleA || contact.penetrationDepth <= 0.0f) return;
    
    // OGC位置修正：基於偏移幾何
    float correctionMagnitude = contact.penetrationDepth * m_positionCorrectionFactor;
    glm::vec3 correction = correctionMagnitude * contact.contactNormal;
    
    if (contact.particleB) {
        // 兩個粒子之間的接觸
        float totalInvMass = contact.particleA->getInverseMass() + contact.particleB->getInverseMass();
        if (totalInvMass > 0.0f) {
            float ratioA = contact.particleA->getInverseMass() / totalInvMass;
            float ratioB = contact.particleB->getInverseMass() / totalInvMass;
            
            contact.particleA->setPosition(contact.particleA->getPosition() + ratioA * correction);
            contact.particleB->setPosition(contact.particleB->getPosition() - ratioB * correction);
        }
    } else {
        // 與靜態物體的接觸
        if (contact.particleA->getInverseMass() > 0.0f) {
            contact.particleA->setPosition(contact.particleA->getPosition() + correction);
        }
    }
}

glm::vec3 OGCContactModel::calculateRelativeVelocity(const OGCContact& contact) {
    if (!contact.particleA) return glm::vec3(0.0f);
    
    glm::vec3 velocityA = contact.particleA->getVelocity();
    glm::vec3 velocityB = contact.particleB ? contact.particleB->getVelocity() : glm::vec3(0.0f);
    
    return velocityA - velocityB;
}

float OGCContactModel::calculateNormalVelocity(const OGCContact& contact) {
    glm::vec3 relativeVelocity = calculateRelativeVelocity(contact);
    return glm::dot(relativeVelocity, contact.contactNormal);
}

} // namespace Physics
