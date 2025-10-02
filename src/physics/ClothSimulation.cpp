#include "physics/ClothSimulation.h"
#include "physics/BulletIntegration.h"
#include <iostream>
#include <cmath>

namespace Physics {

ClothSimulation::ClothSimulation()
    : m_width(0)
    , m_height(0)
    , m_clothSize(2.0f, 2.0f)
    , m_initialPosition(0.0f, 3.0f, 0.0f)
    , m_particleMass(0.1f)
    , m_gravity(0.0f, -9.81f, 0.0f)
    , m_wind(0.0f, 0.0f, 0.0f)
    , m_damping(0.99f)
    , m_structuralStiffness(1000.0f)
    , m_shearStiffness(500.0f)
    , m_bendingStiffness(200.0f)
    , m_constraintIterations(3)
{
}

ClothSimulation::~ClothSimulation() {
    cleanup();
}

bool ClothSimulation::initialize(int width, int height, const glm::vec2& clothSize, 
                                const glm::vec3& position, float particleMass) {
    m_width = width;
    m_height = height;
    m_clothSize = clothSize;
    m_initialPosition = position;
    m_particleMass = particleMass;
    
    // 創建 Bullet Physics 整合
    m_bulletIntegration = std::make_unique<BulletIntegration>();
    
    // 創建 OGC 接觸模型
    m_ogcContactModel = std::make_unique<OGCContactModel>(0.05f, 1000.0f, 50.0f);
    
    // 創建粒子和約束
    createParticles();
    createConstraints();
    
    std::cout << "Cloth simulation initialized: " << width << "x" << height 
              << " particles, " << m_constraints.size() << " constraints" << std::endl;
    
    return true;
}

void ClothSimulation::cleanup() {
    m_particles.clear();
    m_constraints.clear();
    m_contacts.clear();
    m_bulletIntegration.reset();
    m_ogcContactModel.reset();
}

void ClothSimulation::update(float deltaTime) {
    // 1. 應用外力
    applyForces(deltaTime);
    
    // 2. 更新粒子位置 (Verlet 積分)
    updateParticles(deltaTime);
    
    // 3. 求解約束
    for (int i = 0; i < m_constraintIterations; ++i) {
        solveConstraints();
    }
    
    // 4. 處理碰撞
    handleCollisions();
}

void ClothSimulation::addCylinder(const glm::vec3& center, float radius, float height) {
    if (m_bulletIntegration) {
        m_bulletIntegration->addCylinder(center, radius, height);
        std::cout << "Added cylinder: center(" << center.x << ", " << center.y << ", " << center.z 
                  << "), radius=" << radius << ", height=" << height << std::endl;
    }
}

void ClothSimulation::addFloor(const glm::vec3& center, const glm::vec3& size) {
    if (m_bulletIntegration) {
        m_bulletIntegration->addFloor(center, size);
        std::cout << "Added floor: center(" << center.x << ", " << center.y << ", " << center.z 
                  << "), size(" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
    }
}

void ClothSimulation::setParticleFixed(int particleIndex, bool fixed) {
    if (particleIndex >= 0 && particleIndex < static_cast<int>(m_particles.size())) {
        m_particles[particleIndex]->setFixed(fixed);
    }
}

void ClothSimulation::reset() {
    // 重置所有粒子到初始位置
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int index = getParticleIndex(x, y);
            
            float xPos = m_initialPosition.x + (x / float(m_width - 1) - 0.5f) * m_clothSize.x;
            float yPos = m_initialPosition.y;
            float zPos = m_initialPosition.z + (y / float(m_height - 1) - 0.5f) * m_clothSize.y;
            
            glm::vec3 position(xPos, yPos, zPos);
            m_particles[index]->setPosition(position);
            m_particles[index]->setVelocity(glm::vec3(0.0f));
        }
    }
    
    // 清除接觸
    m_contacts.clear();
    
    std::cout << "Cloth simulation reset" << std::endl;
}

void ClothSimulation::createParticles() {
    m_particles.clear();
    m_particles.reserve(m_width * m_height);
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            // 計算粒子位置
            float xPos = m_initialPosition.x + (x / float(m_width - 1) - 0.5f) * m_clothSize.x;
            float yPos = m_initialPosition.y;
            float zPos = m_initialPosition.z + (y / float(m_height - 1) - 0.5f) * m_clothSize.y;
            
            glm::vec3 position(xPos, yPos, zPos);
            
            // 創建粒子
            auto particle = std::make_unique<Particle>(position, m_particleMass);
            
            // 固定頂部邊緣的粒子 (可選)
            if (y == 0) {
                // particle->setFixed(true);
            }
            
            m_particles.push_back(std::move(particle));
            
            // 將粒子添加到 Bullet Physics
            if (m_bulletIntegration) {
                m_bulletIntegration->addParticle(m_particles.back().get(), 0.02f);
            }
        }
    }
}

void ClothSimulation::createConstraints() {
    m_constraints.clear();
    
    float dx = m_clothSize.x / (m_width - 1);
    float dy = m_clothSize.y / (m_height - 1);
    
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int currentIndex = getParticleIndex(x, y);
            
            // 結構約束 (水平和垂直)
            if (x < m_width - 1) {
                int rightIndex = getParticleIndex(x + 1, y);
                m_constraints.emplace_back(currentIndex, rightIndex, dx, m_structuralStiffness);
            }
            
            if (y < m_height - 1) {
                int downIndex = getParticleIndex(x, y + 1);
                m_constraints.emplace_back(currentIndex, downIndex, dy, m_structuralStiffness);
            }
            
            // 剪切約束 (對角線)
            if (x < m_width - 1 && y < m_height - 1) {
                int diagonalIndex = getParticleIndex(x + 1, y + 1);
                float diagonalLength = std::sqrt(dx * dx + dy * dy);
                m_constraints.emplace_back(currentIndex, diagonalIndex, diagonalLength, m_shearStiffness);
            }
            
            if (x > 0 && y < m_height - 1) {
                int diagonalIndex = getParticleIndex(x - 1, y + 1);
                float diagonalLength = std::sqrt(dx * dx + dy * dy);
                m_constraints.emplace_back(currentIndex, diagonalIndex, diagonalLength, m_shearStiffness);
            }
            
            // 彎曲約束 (跳過一個粒子的連接)
            if (x < m_width - 2) {
                int rightIndex = getParticleIndex(x + 2, y);
                m_constraints.emplace_back(currentIndex, rightIndex, 2.0f * dx, m_bendingStiffness);
            }
            
            if (y < m_height - 2) {
                int downIndex = getParticleIndex(x, y + 2);
                m_constraints.emplace_back(currentIndex, downIndex, 2.0f * dy, m_bendingStiffness);
            }
        }
    }
}

void ClothSimulation::applyForces(float deltaTime) {
    // 應用重力
    for (auto& particle : m_particles) {
        if (!particle->isFixed()) {
            particle->addForce(m_gravity * particle->getMass());
        }
    }
    
    // 應用風力 (基於三角形面積)
    for (int y = 0; y < m_height - 1; ++y) {
        for (int x = 0; x < m_width - 1; ++x) {
            // 每個四邊形分成兩個三角形
            int p1 = getParticleIndex(x, y);
            int p2 = getParticleIndex(x + 1, y);
            int p3 = getParticleIndex(x, y + 1);
            int p4 = getParticleIndex(x + 1, y + 1);
            
            // 三角形 1: p1, p2, p3
            glm::vec3 windForce1 = calculateWindForce(*m_particles[p1], *m_particles[p2], *m_particles[p3]);
            m_particles[p1]->addForce(windForce1 / 3.0f);
            m_particles[p2]->addForce(windForce1 / 3.0f);
            m_particles[p3]->addForce(windForce1 / 3.0f);
            
            // 三角形 2: p2, p4, p3
            glm::vec3 windForce2 = calculateWindForce(*m_particles[p2], *m_particles[p4], *m_particles[p3]);
            m_particles[p2]->addForce(windForce2 / 3.0f);
            m_particles[p4]->addForce(windForce2 / 3.0f);
            m_particles[p3]->addForce(windForce2 / 3.0f);
        }
    }
}

void ClothSimulation::updateParticles(float deltaTime) {
    for (auto& particle : m_particles) {
        // Verlet 積分
        particle->update(deltaTime);
        
        // 應用阻尼
        if (!particle->isFixed()) {
            glm::vec3 velocity = particle->getVelocity();
            particle->setVelocity(velocity * m_damping);
        }
    }
    
    // 更新 Bullet Physics 中的粒子位置
    if (m_bulletIntegration) {
        // 這裡需要實現粒子與碰撞物件的對應關係
        // 暫時省略，在實際實現中需要維護這個對應關係
    }
}

void ClothSimulation::solveConstraints() {
    for (const auto& constraint : m_constraints) {
        Particle* particleA = m_particles[constraint.particleA].get();
        Particle* particleB = m_particles[constraint.particleB].get();
        
        glm::vec3 posA = particleA->getPosition();
        glm::vec3 posB = particleB->getPosition();
        
        glm::vec3 delta = posB - posA;
        float currentLength = glm::length(delta);
        
        if (currentLength > 0.0f) {
            float difference = (currentLength - constraint.restLength) / currentLength;
            glm::vec3 correction = delta * difference * 0.5f;
            
            // 根據質量分配修正
            float invMassA = particleA->getInverseMass();
            float invMassB = particleB->getInverseMass();
            float totalInvMass = invMassA + invMassB;
            
            if (totalInvMass > 0.0f) {
                glm::vec3 correctionA = correction * (invMassA / totalInvMass);
                glm::vec3 correctionB = correction * (invMassB / totalInvMass);
                
                if (!particleA->isFixed()) {
                    particleA->setPosition(posA + correctionA);
                }
                if (!particleB->isFixed()) {
                    particleB->setPosition(posB - correctionB);
                }
            }
        }
    }
}

void ClothSimulation::handleCollisions() {
    if (!m_bulletIntegration || !m_ogcContactModel) return;
    
    // 執行碰撞檢測
    m_contacts = m_bulletIntegration->performCollisionDetection();
    
    // 使用 OGC 模型處理接觸
    if (!m_contacts.empty()) {
        m_ogcContactModel->processContacts(m_contacts, 1.0f / 60.0f); // 假設 60 FPS
    }
}

glm::vec3 ClothSimulation::calculateWindForce(const Particle& p1, const Particle& p2, const Particle& p3) {
    if (glm::length(m_wind) == 0.0f) return glm::vec3(0.0f);
    
    // 計算三角形法線
    glm::vec3 v1 = p2.getPosition() - p1.getPosition();
    glm::vec3 v2 = p3.getPosition() - p1.getPosition();
    glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
    
    // 計算三角形面積
    float area = 0.5f * glm::length(glm::cross(v1, v2));
    
    // 風力與法線的點積決定風力大小
    float windEffect = glm::dot(glm::normalize(m_wind), normal);
    
    return m_wind * windEffect * area;
}

} // namespace Physics
