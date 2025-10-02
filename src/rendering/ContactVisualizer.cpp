#include "rendering/ContactVisualizer.h"
#include "rendering/Shader.h"
#include "physics/OGCContactModel.h"
#include <iostream>
#include <cmath>

namespace Rendering {

ContactVisualizer::ContactVisualizer()
    : m_pointVAO(0)
    , m_pointVBO(0)
    , m_lineVAO(0)
    , m_lineVBO(0)
    , m_sphereVAO(0)
    , m_sphereVBO(0)
    , m_sphereEBO(0)
    , m_showContactPoints(true)
    , m_showContactNormals(true)
    , m_showContactForces(true)
    , m_showOffsetGeometry(false)
{
}

ContactVisualizer::~ContactVisualizer() {
    cleanup();
}

bool ContactVisualizer::initialize() {
    // 創建著色器
    createShaders();
    
    // 創建幾何體
    createGeometry();
    
    std::cout << "ContactVisualizer initialized" << std::endl;
    return true;
}

void ContactVisualizer::cleanup() {
    // 清理 OpenGL 物件
    if (m_pointVAO) glDeleteVertexArrays(1, &m_pointVAO);
    if (m_pointVBO) glDeleteBuffers(1, &m_pointVBO);
    if (m_lineVAO) glDeleteVertexArrays(1, &m_lineVAO);
    if (m_lineVBO) glDeleteBuffers(1, &m_lineVBO);
    if (m_sphereVAO) glDeleteVertexArrays(1, &m_sphereVAO);
    if (m_sphereVBO) glDeleteBuffers(1, &m_sphereVBO);
    if (m_sphereEBO) glDeleteBuffers(1, &m_sphereEBO);
    
    m_pointVAO = m_pointVBO = 0;
    m_lineVAO = m_lineVBO = 0;
    m_sphereVAO = m_sphereVBO = m_sphereEBO = 0;
}

void ContactVisualizer::renderContacts(const std::vector<Physics::OGCContact>& contacts,
                                      const glm::mat4& viewMatrix,
                                      const glm::mat4& projectionMatrix) {
    if (contacts.empty()) return;
    
    // 渲染接觸點
    if (m_showContactPoints) {
        renderContactPoints(contacts, viewMatrix, projectionMatrix);
    }
    
    // 渲染接觸法線
    if (m_showContactNormals) {
        renderContactNormals(contacts, viewMatrix, projectionMatrix);
    }
    
    // 渲染接觸力
    if (m_showContactForces) {
        renderContactForces(contacts, viewMatrix, projectionMatrix);
    }
    
    // 渲染 OGC 偏移幾何
    if (m_showOffsetGeometry) {
        renderOffsetGeometry(contacts, viewMatrix, projectionMatrix);
    }
}

void ContactVisualizer::renderContactPoints(const std::vector<Physics::OGCContact>& contacts,
                                           const glm::mat4& viewMatrix,
                                           const glm::mat4& projectionMatrix,
                                           float pointSize,
                                           const glm::vec3& color) {
    if (!m_pointShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        renderSphere(contact.contactPoint, pointSize, viewMatrix, projectionMatrix, color);
    }
}

void ContactVisualizer::renderContactNormals(const std::vector<Physics::OGCContact>& contacts,
                                            const glm::mat4& viewMatrix,
                                            const glm::mat4& projectionMatrix,
                                            float length,
                                            const glm::vec3& color) {
    if (!m_lineShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        glm::vec3 start = contact.contactPoint;
        glm::vec3 end = start + contact.contactNormal * length;
        renderLine(start, end, viewMatrix, projectionMatrix, color);
    }
}

void ContactVisualizer::renderContactForces(const std::vector<Physics::OGCContact>& contacts,
                                           const glm::mat4& viewMatrix,
                                           const glm::mat4& projectionMatrix,
                                           float forceScale,
                                           const glm::vec3& color) {
    if (!m_lineShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        if (contact.contactForce > 0.0f) {
            glm::vec3 start = contact.contactPoint;
            glm::vec3 forceVector = contact.forceDirection * contact.contactForce * forceScale;
            glm::vec3 end = start + forceVector;
            
            // 使用不同的顏色表示力的大小
            float normalizedForce = std::min(contact.contactForce / 1000.0f, 1.0f);
            glm::vec3 forceColor = glm::mix(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), normalizedForce);
            
            renderLine(start, end, viewMatrix, projectionMatrix, forceColor);
        }
    }
}

void ContactVisualizer::renderOffsetGeometry(const std::vector<Physics::OGCContact>& contacts,
                                            const glm::mat4& viewMatrix,
                                            const glm::mat4& projectionMatrix,
                                            const glm::vec3& color) {
    if (!m_lineShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        if (glm::length(contact.offsetGeometry) > 0.0f) {
            glm::vec3 start = contact.contactPoint;
            glm::vec3 end = start + contact.offsetGeometry;
            renderLine(start, end, viewMatrix, projectionMatrix, color);
        }
    }
}

void ContactVisualizer::createShaders() {
    // 點/球體著色器
    std::string pointVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    std::string pointFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    // 線段著色器
    std::string lineVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * vec4(aPos, 1.0);
        }
    )";
    
    std::string lineFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    m_pointShader = std::make_shared<Shader>(pointVertexShader, pointFragmentShader);
    m_lineShader = std::make_shared<Shader>(lineVertexShader, lineFragmentShader);
    
    if (!m_pointShader->isValid() || !m_lineShader->isValid()) {
        std::cerr << "Failed to create contact visualizer shaders" << std::endl;
    }
}

void ContactVisualizer::createGeometry() {
    // 創建小球體用於接觸點
    createSphere(1.0f, 8);
    
    // 創建線段 VAO
    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);
    
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void ContactVisualizer::createSphere(float radius, int segments) {
    m_sphereVertices.clear();
    m_sphereIndices.clear();
    
    // 生成球體頂點
    for (int i = 0; i <= segments; ++i) {
        float phi = M_PI * i / segments;
        for (int j = 0; j <= segments; ++j) {
            float theta = 2.0f * M_PI * j / segments;
            
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            
            m_sphereVertices.push_back(x);
            m_sphereVertices.push_back(y);
            m_sphereVertices.push_back(z);
        }
    }
    
    // 生成球體索引
    for (int i = 0; i < segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            int first = i * (segments + 1) + j;
            int second = first + segments + 1;
            
            m_sphereIndices.push_back(first);
            m_sphereIndices.push_back(second);
            m_sphereIndices.push_back(first + 1);
            
            m_sphereIndices.push_back(second);
            m_sphereIndices.push_back(second + 1);
            m_sphereIndices.push_back(first + 1);
        }
    }
    
    // 創建 OpenGL 物件
    glGenVertexArrays(1, &m_sphereVAO);
    glGenBuffers(1, &m_sphereVBO);
    glGenBuffers(1, &m_sphereEBO);
    
    glBindVertexArray(m_sphereVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, m_sphereVertices.size() * sizeof(float), 
                 m_sphereVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_sphereIndices.size() * sizeof(unsigned int), 
                 m_sphereIndices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void ContactVisualizer::renderLine(const glm::vec3& start, const glm::vec3& end,
                                  const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                  const glm::vec3& color) {
    if (!m_lineShader) return;
    
    float lineData[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };
    
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineData), lineData);
    
    m_lineShader->use();
    m_lineShader->setMatrix4("view", viewMatrix);
    m_lineShader->setMatrix4("projection", projectionMatrix);
    m_lineShader->setVector3("color", color);
    
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

void ContactVisualizer::renderSphere(const glm::vec3& position, float scale,
                                     const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                     const glm::vec3& color) {
    if (!m_pointShader || m_sphereIndices.empty()) return;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));
    
    m_pointShader->use();
    m_pointShader->setMatrix4("model", model);
    m_pointShader->setMatrix4("view", viewMatrix);
    m_pointShader->setMatrix4("projection", projectionMatrix);
    m_pointShader->setVector3("color", color);
    
    glBindVertexArray(m_sphereVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_sphereIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace Rendering
