#include "rendering/OpenGLRenderer.h"
#include "rendering/Shader.h"
#include "rendering/Camera.h"
#include "rendering/ContactVisualizer.h"
#include "physics/Particle.h"
#include "physics/OGCContactModel.h"
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Rendering {

OpenGLRenderer::OpenGLRenderer()
    : m_window(nullptr)
    , m_windowWidth(1200)
    , m_windowHeight(800)
    , m_sphereVAO(0), m_sphereVBO(0), m_sphereEBO(0)
    , m_cylinderVAO(0), m_cylinderVBO(0), m_cylinderEBO(0)
    , m_floorVAO(0), m_floorVBO(0), m_floorEBO(0)
    , m_lineVAO(0), m_lineVBO(0)
{
}

OpenGLRenderer::~OpenGLRenderer() {
    cleanup();
}

bool OpenGLRenderer::initialize(int width, int height, const char* title) {
    m_windowWidth = width;
    m_windowHeight = height;
    
    // 設定 GLFW 錯誤回調
    glfwSetErrorCallback(glfwErrorCallback);
    
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // 設定 OpenGL 版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // 創建視窗
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    
    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    // 初始化 OpenGL 設定
    initializeOpenGL();
    
    // 創建著色器
    createShaders();
    
    // 創建幾何體
    createGeometry();
    
    // 創建相機
    m_camera = std::make_shared<Camera>(
        glm::vec3(0.0f, 3.0f, 5.0f),  // 位置
        glm::vec3(0.0f, 0.0f, 0.0f),  // 目標
        glm::vec3(0.0f, 1.0f, 0.0f)   // 上方向
    );
    
    // 創建接觸可視化器
    m_contactVisualizer = std::make_shared<ContactVisualizer>();
    m_contactVisualizer->initialize();
    
    std::cout << "OpenGL Renderer initialized: " << width << "x" << height << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    return true;
}

void OpenGLRenderer::cleanup() {
    // 清理 OpenGL 物件
    if (m_sphereVAO) glDeleteVertexArrays(1, &m_sphereVAO);
    if (m_sphereVBO) glDeleteBuffers(1, &m_sphereVBO);
    if (m_sphereEBO) glDeleteBuffers(1, &m_sphereEBO);
    if (m_cylinderVAO) glDeleteVertexArrays(1, &m_cylinderVAO);
    if (m_cylinderVBO) glDeleteBuffers(1, &m_cylinderVBO);
    if (m_cylinderEBO) glDeleteBuffers(1, &m_cylinderEBO);
    if (m_floorVAO) glDeleteVertexArrays(1, &m_floorVAO);
    if (m_floorVBO) glDeleteBuffers(1, &m_floorVBO);
    if (m_floorEBO) glDeleteBuffers(1, &m_floorEBO);
    if (m_lineVAO) glDeleteVertexArrays(1, &m_lineVAO);
    if (m_lineVBO) glDeleteBuffers(1, &m_lineVBO);
    
    // 清理 GLFW
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void OpenGLRenderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::endFrame() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool OpenGLRenderer::shouldClose() {
    return glfwWindowShouldClose(m_window);
}

void OpenGLRenderer::processInput() {
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
    
    // 相機控制
    if (m_camera) {
        static double lastX = m_windowWidth / 2.0;
        static double lastY = m_windowHeight / 2.0;
        static bool firstMouse = true;
        
        double xpos, ypos;
        glfwGetCursorPos(m_window, &xpos, &ypos);
        
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Y座標反轉
        lastX = xpos;
        lastY = ypos;
        
        // 滑鼠左鍵拖拽旋轉相機
        if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            m_camera->processMouseMovement(xoffset, yoffset);
        }
        
        // 滑鼠滾輪縮放
        static double lastScrollY = 0.0;
        double scrollX, scrollY;
        glfwGetScrollOffset(m_window, &scrollX, &scrollY);
        if (scrollY != lastScrollY) {
            m_camera->processMouseScroll(scrollY - lastScrollY);
            lastScrollY = scrollY;
        }
        
        // 鍵盤控制
        float deltaTime = 1.0f / 60.0f; // 假設 60 FPS
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
            m_camera->processKeyboard(GLFW_KEY_W, deltaTime);
        }
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
            m_camera->processKeyboard(GLFW_KEY_S, deltaTime);
        }
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
            m_camera->processKeyboard(GLFW_KEY_A, deltaTime);
        }
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
            m_camera->processKeyboard(GLFW_KEY_D, deltaTime);
        }
        if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS) {
            m_camera->processKeyboard(GLFW_KEY_Q, deltaTime);
        }
        if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS) {
            m_camera->processKeyboard(GLFW_KEY_E, deltaTime);
        }
        if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
            m_camera->reset();
        }
    }
}

void OpenGLRenderer::renderClothParticles(const std::vector<Physics::Particle*>& particles) {
    if (!m_basicShader || particles.empty()) return;
    
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix(
        static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight)
    );
    
    m_basicShader->use();
    m_basicShader->setMatrix4("view", view);
    m_basicShader->setMatrix4("projection", projection);
    m_basicShader->setVector3("color", glm::vec3(0.2f, 0.6f, 1.0f)); // 藍色粒子
    
    glBindVertexArray(m_sphereVAO);
    
    for (const auto& particle : particles) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, particle->getPosition());
        model = glm::scale(model, glm::vec3(0.02f)); // 小球體
        
        m_basicShader->setMatrix4("model", model);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_sphereIndices.size()), GL_UNSIGNED_INT, 0);
    }
    
    glBindVertexArray(0);
}

void OpenGLRenderer::renderClothConstraints(const std::vector<Physics::Particle*>& particles,
                                           const std::vector<std::pair<int, int>>& constraints) {
    if (!m_lineShader || particles.empty() || constraints.empty()) return;
    
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix(
        static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight)
    );
    
    m_lineShader->use();
    m_lineShader->setMatrix4("view", view);
    m_lineShader->setMatrix4("projection", projection);
    m_lineShader->setVector3("color", glm::vec3(0.8f, 0.8f, 0.8f)); // 灰色線條
    
    glBindVertexArray(m_lineVAO);
    
    for (const auto& constraint : constraints) {
        if (constraint.first < particles.size() && constraint.second < particles.size()) {
            glm::vec3 pos1 = particles[constraint.first]->getPosition();
            glm::vec3 pos2 = particles[constraint.second]->getPosition();
            
            float lineData[] = {
                pos1.x, pos1.y, pos1.z,
                pos2.x, pos2.y, pos2.z
            };
            
            glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineData), lineData);
            
            glDrawArrays(GL_LINES, 0, 2);
        }
    }
    
    glBindVertexArray(0);
}

void OpenGLRenderer::renderCylinder(const glm::vec3& center, float radius, float height, const glm::vec3& color) {
    if (!m_basicShader) return;
    
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix(
        static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight)
    );
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, center);
    model = glm::scale(model, glm::vec3(radius, height * 0.5f, radius));
    
    m_basicShader->use();
    m_basicShader->setMatrix4("model", model);
    m_basicShader->setMatrix4("view", view);
    m_basicShader->setMatrix4("projection", projection);
    m_basicShader->setVector3("color", color);
    
    glBindVertexArray(m_cylinderVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_cylinderIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void OpenGLRenderer::renderFloor(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color) {
    if (!m_basicShader) return;
    
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix(
        static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight)
    );
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, center);
    model = glm::scale(model, size);
    
    m_basicShader->use();
    m_basicShader->setMatrix4("model", model);
    m_basicShader->setMatrix4("view", view);
    m_basicShader->setMatrix4("projection", projection);
    m_basicShader->setVector3("color", color);
    
    glBindVertexArray(m_floorVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_floorIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void OpenGLRenderer::renderContacts(const std::vector<Physics::OGCContact>& contacts) {
    if (!m_contactVisualizer || contacts.empty()) return;
    
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix(
        static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight)
    );
    
    m_contactVisualizer->renderContacts(contacts, view, projection);
}

void OpenGLRenderer::setWindowSize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    glViewport(0, 0, width, height);
}

void OpenGLRenderer::initializeOpenGL() {
    // 啟用深度測試
    glEnable(GL_DEPTH_TEST);
    
    // 設定背景顏色
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // 啟用線條平滑
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // 設定線條寬度
    glLineWidth(2.0f);
    
    // 設定視口
    glViewport(0, 0, m_windowWidth, m_windowHeight);
}

void OpenGLRenderer::createShaders() {
    // 基本著色器 (用於實體物件)
    std::string basicVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    std::string basicFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    // 線條著色器
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
    
    m_basicShader = std::make_shared<Shader>(basicVertexShader, basicFragmentShader);
    m_lineShader = std::make_shared<Shader>(lineVertexShader, lineFragmentShader);
    
    if (!m_basicShader->isValid() || !m_lineShader->isValid()) {
        std::cerr << "Failed to create renderer shaders" << std::endl;
    }
}

void OpenGLRenderer::createGeometry() {
    createSphere();
    createCylinder();
    createFloor();
    
    // 創建線條 VAO
    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);
    
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void OpenGLRenderer::createSphere(float radius, int segments) {
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

void OpenGLRenderer::createCylinder(float radius, float height, int segments) {
    m_cylinderVertices.clear();
    m_cylinderIndices.clear();
    
    // 頂部和底部圓心
    m_cylinderVertices.push_back(0.0f); m_cylinderVertices.push_back(height); m_cylinderVertices.push_back(0.0f);
    m_cylinderVertices.push_back(0.0f); m_cylinderVertices.push_back(-height); m_cylinderVertices.push_back(0.0f);
    
    // 頂部和底部圓周頂點
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * M_PI * i / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        // 頂部
        m_cylinderVertices.push_back(x);
        m_cylinderVertices.push_back(height);
        m_cylinderVertices.push_back(z);
        
        // 底部
        m_cylinderVertices.push_back(x);
        m_cylinderVertices.push_back(-height);
        m_cylinderVertices.push_back(z);
    }
    
    // 生成索引
    // 側面
    for (int i = 0; i < segments; ++i) {
        int topCurrent = 2 + i * 2;
        int topNext = 2 + ((i + 1) % segments) * 2;
        int bottomCurrent = topCurrent + 1;
        int bottomNext = topNext + 1;
        
        // 第一個三角形
        m_cylinderIndices.push_back(topCurrent);
        m_cylinderIndices.push_back(bottomCurrent);
        m_cylinderIndices.push_back(topNext);
        
        // 第二個三角形
        m_cylinderIndices.push_back(topNext);
        m_cylinderIndices.push_back(bottomCurrent);
        m_cylinderIndices.push_back(bottomNext);
    }
    
    // 頂部和底部
    for (int i = 0; i < segments; ++i) {
        int current = 2 + i * 2;
        int next = 2 + ((i + 1) % segments) * 2;
        
        // 頂部
        m_cylinderIndices.push_back(0);
        m_cylinderIndices.push_back(current);
        m_cylinderIndices.push_back(next);
        
        // 底部
        m_cylinderIndices.push_back(1);
        m_cylinderIndices.push_back(next + 1);
        m_cylinderIndices.push_back(current + 1);
    }
    
    // 創建 OpenGL 物件
    glGenVertexArrays(1, &m_cylinderVAO);
    glGenBuffers(1, &m_cylinderVBO);
    glGenBuffers(1, &m_cylinderEBO);
    
    glBindVertexArray(m_cylinderVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, m_cylinderVertices.size() * sizeof(float), 
                 m_cylinderVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cylinderEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_cylinderIndices.size() * sizeof(unsigned int), 
                 m_cylinderIndices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void OpenGLRenderer::createFloor() {
    m_floorVertices = {
        // 位置
        -1.0f, 0.0f, -1.0f,
         1.0f, 0.0f, -1.0f,
         1.0f, 0.0f,  1.0f,
        -1.0f, 0.0f,  1.0f
    };
    
    m_floorIndices = {
        0, 1, 2,
        2, 3, 0
    };
    
    // 創建 OpenGL 物件
    glGenVertexArrays(1, &m_floorVAO);
    glGenBuffers(1, &m_floorVBO);
    glGenBuffers(1, &m_floorEBO);
    
    glBindVertexArray(m_floorVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_floorVBO);
    glBufferData(GL_ARRAY_BUFFER, m_floorVertices.size() * sizeof(float), 
                 m_floorVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_floorIndices.size() * sizeof(unsigned int), 
                 m_floorIndices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void OpenGLRenderer::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void OpenGLRenderer::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    OpenGLRenderer* renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    if (renderer) {
        renderer->setWindowSize(width, height);
    }
}

} // namespace Rendering
