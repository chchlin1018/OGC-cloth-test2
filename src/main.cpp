#include <iostream>
#include <memory>
#include <chrono>
#include <vector>

#include "rendering/OpenGLRenderer.h"
#include "physics/ClothSimulation.h"
#include "physics/Particle.h"

/**
 * @brief OGC 布料模擬主程序
 * 
 * 展示布料從高處掉落到圓柱體，然後落到地板的完整物理過程。
 * 使用 OGC Contact Model 進行精確的接觸檢測和力計算。
 * 可視化接觸點、接觸法線和接觸力的大小與方向。
 */

class ClothSimulationApp {
public:
    ClothSimulationApp() 
        : m_renderer(nullptr)
        , m_clothSimulation(nullptr)
        , m_isRunning(false)
        , m_isPaused(false)
        , m_showWireframe(true)
        , m_showParticles(true)
        , m_showContacts(true)
        , m_timeAccumulator(0.0f)
        , m_fixedTimeStep(1.0f / 60.0f)
    {
    }

    ~ClothSimulationApp() {
        cleanup();
    }

    bool initialize() {
        std::cout << "=== OGC 布料模擬程序 ===" << std::endl;
        std::cout << "初始化渲染器..." << std::endl;
        
        // 創建渲染器
        m_renderer = std::make_unique<Rendering::OpenGLRenderer>();
        if (!m_renderer->initialize(1200, 800, "OGC Cloth Simulation Test")) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }
        
        std::cout << "初始化布料模擬..." << std::endl;
        
        // 創建布料模擬
        m_clothSimulation = std::make_unique<Physics::ClothSimulation>();
        if (!m_clothSimulation->initialize(20, 20, glm::vec2(2.0f, 2.0f), glm::vec3(0.0f, 3.0f, 0.0f))) {
            std::cerr << "Failed to initialize cloth simulation" << std::endl;
            return false;
        }
        
        // 設定物理參數
        m_clothSimulation->setGravity(glm::vec3(0.0f, -9.81f, 0.0f));
        m_clothSimulation->setWind(glm::vec3(0.5f, 0.0f, 0.2f));
        m_clothSimulation->setDamping(0.99f);
        
        // 固定布料的頂部邊緣
        auto clothSize = m_clothSimulation->getClothSize();
        for (int x = 0; x < clothSize.first; ++x) {
            int topIndex = x; // 頂部行的索引
            m_clothSimulation->setParticleFixed(topIndex, true);
        }
        
        // 添加碰撞體
        m_clothSimulation->addCylinder(glm::vec3(0.0f, 1.0f, 0.0f), 0.5f, 1.0f);
        m_clothSimulation->addFloor(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(5.0f, 0.1f, 5.0f));
        
        std::cout << "初始化完成！" << std::endl;
        printControls();
        
        return true;
    }

    void run() {
        if (!m_renderer || !m_clothSimulation) {
            std::cerr << "Application not properly initialized" << std::endl;
            return;
        }
        
        m_isRunning = true;
        auto lastTime = std::chrono::high_resolution_clock::now();
        
        std::cout << "開始模擬..." << std::endl;
        
        while (m_isRunning && !m_renderer->shouldClose()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // 處理輸入
            processInput();
            
            // 更新物理模擬 (固定時間步長)
            if (!m_isPaused) {
                m_timeAccumulator += deltaTime;
                while (m_timeAccumulator >= m_fixedTimeStep) {
                    m_clothSimulation->update(m_fixedTimeStep);
                    m_timeAccumulator -= m_fixedTimeStep;
                }
            }
            
            // 渲染
            render();
            
            // 限制幀率
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
        
        std::cout << "模擬結束" << std::endl;
    }

private:
    void processInput() {
        m_renderer->processInput();
        
        // 檢查鍵盤輸入
        GLFWwindow* window = m_renderer->getWindow();
        
        // 暫停/繼續
        static bool spacePressed = false;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
            m_isPaused = !m_isPaused;
            std::cout << (m_isPaused ? "模擬暫停" : "模擬繼續") << std::endl;
            spacePressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
            spacePressed = false;
        }
        
        // 重置場景
        static bool rPressed = false;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
            m_clothSimulation->reset();
            std::cout << "場景重置" << std::endl;
            rPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            rPressed = false;
        }
        
        // 切換線框模式
        static bool wPressed = false;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !wPressed) {
            m_showWireframe = !m_showWireframe;
            std::cout << "線框模式: " << (m_showWireframe ? "開啟" : "關閉") << std::endl;
            wPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
            wPressed = false;
        }
        
        // 切換粒子顯示
        static bool pPressed = false;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pPressed) {
            m_showParticles = !m_showParticles;
            std::cout << "粒子顯示: " << (m_showParticles ? "開啟" : "關閉") << std::endl;
            pPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            pPressed = false;
        }
        
        // 切換接觸點顯示
        static bool cPressed = false;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cPressed) {
            m_showContacts = !m_showContacts;
            std::cout << "接觸點顯示: " << (m_showContacts ? "開啟" : "關閉") << std::endl;
            cPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
            cPressed = false;
        }
        
        // 退出程序
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            m_isRunning = false;
        }
    }

    void render() {
        m_renderer->beginFrame();
        
        // 渲染布料粒子
        if (m_showParticles) {
            const auto& particles = m_clothSimulation->getParticles();
            std::vector<Physics::Particle*> particlePtrs;
            for (const auto& particle : particles) {
                particlePtrs.push_back(particle.get());
            }
            m_renderer->renderClothParticles(particlePtrs);
        }
        
        // 渲染布料約束 (線框)
        if (m_showWireframe) {
            const auto& particles = m_clothSimulation->getParticles();
            const auto& constraints = m_clothSimulation->getConstraints();
            
            std::vector<Physics::Particle*> particlePtrs;
            for (const auto& particle : particles) {
                particlePtrs.push_back(particle.get());
            }
            
            std::vector<std::pair<int, int>> constraintPairs;
            for (const auto& constraint : constraints) {
                constraintPairs.emplace_back(constraint.particleA, constraint.particleB);
            }
            
            m_renderer->renderClothConstraints(particlePtrs, constraintPairs);
        }
        
        // 渲染碰撞體
        m_renderer->renderCylinder(glm::vec3(0.0f, 1.0f, 0.0f), 0.5f, 1.0f, glm::vec3(0.8f, 0.3f, 0.3f));
        m_renderer->renderFloor(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(5.0f, 0.1f, 5.0f), glm::vec3(0.3f, 0.8f, 0.3f));
        
        // 渲染接觸點和接觸力
        if (m_showContacts) {
            const auto& contacts = m_clothSimulation->getContacts();
            m_renderer->renderContacts(contacts);
        }
        
        m_renderer->endFrame();
    }

    void cleanup() {
        m_clothSimulation.reset();
        m_renderer.reset();
    }

    void printControls() {
        std::cout << "\n=== 控制說明 ===" << std::endl;
        std::cout << "滑鼠左鍵拖拽: 旋轉相機" << std::endl;
        std::cout << "滑鼠滾輪:     縮放" << std::endl;
        std::cout << "WASD:         移動相機" << std::endl;
        std::cout << "QE:           上下移動相機" << std::endl;
        std::cout << "空格鍵:       暫停/繼續模擬" << std::endl;
        std::cout << "R:            重置場景" << std::endl;
        std::cout << "W:            切換線框模式" << std::endl;
        std::cout << "P:            切換粒子顯示" << std::endl;
        std::cout << "C:            切換接觸點顯示" << std::endl;
        std::cout << "ESC:          退出程序" << std::endl;
        std::cout << "==================\n" << std::endl;
    }

private:
    std::unique_ptr<Rendering::OpenGLRenderer> m_renderer;
    std::unique_ptr<Physics::ClothSimulation> m_clothSimulation;
    
    bool m_isRunning;
    bool m_isPaused;
    bool m_showWireframe;
    bool m_showParticles;
    bool m_showContacts;
    
    float m_timeAccumulator;
    float m_fixedTimeStep;
};

int main() {
    try {
        ClothSimulationApp app;
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return -1;
        }
        
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return -1;
    }
    
    return 0;
}
