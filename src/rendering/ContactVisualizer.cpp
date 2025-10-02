#include "rendering/ContactVisualizer.h"
#include "rendering/Shader.h"
#include "physics/OGCContactModel.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

namespace Rendering {

ContactVisualizer::ContactVisualizer()
    : m_pointVAO(0), m_pointVBO(0)
    , m_lineVAO(0), m_lineVBO(0)
    , m_sphereVAO(0), m_sphereVBO(0), m_sphereEBO(0) {
}

ContactVisualizer::~ContactVisualizer() {
    cleanup();
}

void ContactVisualizer::initialize() {
    initializeShaders();
    initializeGeometry();
}

void ContactVisualizer::cleanup() {
    if (m_pointVAO) {
        glDeleteVertexArrays(1, &m_pointVAO);
        glDeleteBuffers(1, &m_pointVBO);
        m_pointVAO = m_pointVBO = 0;
    }
    
    if (m_lineVAO) {
        glDeleteVertexArrays(1, &m_lineVAO);
        glDeleteBuffers(1, &m_lineVBO);
        m_lineVAO = m_lineVBO = 0;
    }
    
    if (m_sphereVAO) {
        glDeleteVertexArrays(1, &m_sphereVAO);
        glDeleteBuffers(1, &m_sphereVBO);
        glDeleteBuffers(1, &m_sphereEBO);
        m_sphereVAO = m_sphereVBO = m_sphereEBO = 0;
    }
}

void ContactVisualizer::renderContactPoints(const std::vector<::Physics::OGCContact>& contacts,
                                           const glm::mat4& view, const glm::mat4& projection) {
    if (!m_pointShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        renderSphere(contact.contactPoint, 0.02f, view, projection, glm::vec3(1.0f, 0.0f, 0.0f));
    }
}

void ContactVisualizer::renderContactNormals(const std::vector<::Physics::OGCContact>& contacts,
                                            const glm::mat4& view, const glm::mat4& projection) {
    if (!m_lineShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        glm::vec3 start = contact.contactPoint;
        glm::vec3 end = start + contact.contactNormal * 0.1f;
        renderLine(start, end, view, projection, glm::vec3(0.0f, 0.0f, 1.0f));
    }
}

void ContactVisualizer::renderContactForces(const std::vector<::Physics::OGCContact>& contacts,
                                           const glm::mat4& view, const glm::mat4& projection) {
    if (!m_lineShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        if (contact.contactForce > 0.001f) {
            glm::vec3 start = contact.contactPoint;
            glm::vec3 end = start + contact.forceDirection * 0.15f;
            
            // 根據力的大小設定顏色
            float forceMagnitude = contact.contactForce;
            glm::vec3 color = glm::vec3(forceMagnitude * 0.1f, 1.0f - forceMagnitude * 0.1f, 0.0f);
            
            renderLine(start, end, view, projection, color);
        }
    }
}

void ContactVisualizer::renderOffsetGeometry(const std::vector<::Physics::OGCContact>& contacts,
                                            const glm::mat4& view, const glm::mat4& projection) {
    if (!m_lineShader || contacts.empty()) return;
    
    for (const auto& contact : contacts) {
        if (glm::length(contact.offsetGeometry) > 0.0f) {
            glm::vec3 start = contact.contactPoint;
            glm::vec3 end = start + contact.offsetGeometry;
            renderLine(start, end, view, projection, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
}

void ContactVisualizer::initializeShaders() {
    // 點著色器
    const char* pointVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    const char* pointFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    // 線著色器
    const char* lineVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * vec4(aPos, 1.0);
        }
    )";
    
    const char* lineFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    try {
        m_pointShader = std::make_unique<Shader>(pointVertexShader, pointFragmentShader);
        m_lineShader = std::make_unique<Shader>(lineVertexShader, lineFragmentShader);
        m_sphereShader = std::make_unique<Shader>(pointVertexShader, pointFragmentShader);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create ContactVisualizer shaders: " << e.what() << std::endl;
    }
}

void ContactVisualizer::initializeGeometry() {
    // 初始化點 VAO
    glGenVertexArrays(1, &m_pointVAO);
    glGenBuffers(1, &m_pointVBO);
    
    // 初始化線 VAO
    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);
    
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 生成球體幾何
    generateSphere();
    
    // 初始化球體 VAO
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

void ContactVisualizer::generateSphere(float radius, int sectorCount, int stackCount) {
    m_sphereVertices.clear();
    m_sphereIndices.clear();
    
    float x, y, z, xy;
    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;
    
    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);
        
        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            
            m_sphereVertices.push_back(x);
            m_sphereVertices.push_back(y);
            m_sphereVertices.push_back(z);
        }
    }
    
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;
        
        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                m_sphereIndices.push_back(k1);
                m_sphereIndices.push_back(k2);
                m_sphereIndices.push_back(k1 + 1);
            }
            
            if (i != (stackCount - 1)) {
                m_sphereIndices.push_back(k1 + 1);
                m_sphereIndices.push_back(k2);
                m_sphereIndices.push_back(k2 + 1);
            }
        }
    }
}

void ContactVisualizer::renderLine(const glm::vec3& start, const glm::vec3& end,
                                  const glm::mat4& view, const glm::mat4& projection,
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
    m_lineShader->setMat4("view", view);
    m_lineShader->setMat4("projection", projection);
    m_lineShader->setVec3("color", color);
    
    glDrawArrays(GL_LINES, 0, 2);
}

void ContactVisualizer::renderSphere(const glm::vec3& position, float scale,
                                     const glm::mat4& view, const glm::mat4& projection,
                                     const glm::vec3& color) {
    if (!m_sphereShader) return;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));
    
    m_sphereShader->use();
    m_sphereShader->setMat4("model", model);
    m_sphereShader->setMat4("view", view);
    m_sphereShader->setMat4("projection", projection);
    m_sphereShader->setVec3("color", color);
    
    glBindVertexArray(m_sphereVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_sphereIndices.size()), GL_UNSIGNED_INT, 0);
}

} // namespace Rendering
