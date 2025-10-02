#include "physics/Particle.h"
#include <algorithm>

namespace Physics {

Particle::Particle(const glm::vec3& position, float mass)
    : m_position(position)
    , m_previousPosition(position)
    , m_force(0.0f)
    , m_mass(mass)
    , m_inverseMass(mass > 0.0f ? 1.0f / mass : 0.0f)
{
}

void Particle::update(float deltaTime) {
    if (isFixed()) {
        // 固定粒子不更新位置
        clearForces();
        return;
    }
    
    // Verlet 積分
    glm::vec3 acceleration = m_force * m_inverseMass;
    glm::vec3 newPosition = 2.0f * m_position - m_previousPosition + acceleration * deltaTime * deltaTime;
    
    // 更新位置
    m_previousPosition = m_position;
    m_position = newPosition;
    
    // 清除力
    clearForces();
}

void Particle::addForce(const glm::vec3& force) {
    m_force += force;
}

void Particle::clearForces() {
    m_force = glm::vec3(0.0f);
}

void Particle::setPosition(const glm::vec3& position) {
    m_position = position;
}

glm::vec3 Particle::getVelocity() const {
    // 使用 Verlet 積分計算速度
    return (m_position - m_previousPosition);
}

void Particle::setVelocity(const glm::vec3& velocity) {
    // 通過調整上一幀位置來設定速度
    m_previousPosition = m_position - velocity;
}

void Particle::setMass(float mass) {
    m_mass = mass;
    m_inverseMass = mass > 0.0f ? 1.0f / mass : 0.0f;
}

void Particle::setFixed(bool fixed) {
    if (fixed) {
        m_inverseMass = 0.0f;
    } else {
        m_inverseMass = m_mass > 0.0f ? 1.0f / m_mass : 0.0f;
    }
}

} // namespace Physics
