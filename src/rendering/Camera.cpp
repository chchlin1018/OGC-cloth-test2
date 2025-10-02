#include "rendering/Camera.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

namespace Rendering {

Camera::Camera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    : m_position(position)
    , m_target(target)
    , m_up(up)
    , m_initialPosition(position)
    , m_initialTarget(target)
    , m_initialUp(up)
    , m_minDistance(1.0f)
    , m_maxDistance(50.0f)
    , m_minElevation(-89.0f)
    , m_maxElevation(89.0f)
{
    // 計算初始球座標
    glm::vec3 direction = m_position - m_target;
    m_distance = glm::length(direction);
    
    if (m_distance > 0.0f) {
        direction = glm::normalize(direction);
        m_elevation = glm::degrees(asin(direction.y));
        m_azimuth = glm::degrees(atan2(direction.x, direction.z));
    } else {
        m_distance = 10.0f;
        m_elevation = 0.0f;
        m_azimuth = 0.0f;
    }
    
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect, float fov, float nearPlane, float farPlane) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

void Camera::processMouseMovement(float xOffset, float yOffset, float sensitivity) {
    // 更新方位角和仰角
    m_azimuth += xOffset * sensitivity * 57.2958f; // 轉換為度
    m_elevation -= yOffset * sensitivity * 57.2958f; // Y軸反轉
    
    // 約束仰角
    m_elevation = std::clamp(m_elevation, m_minElevation, m_maxElevation);
    
    // 更新位置
    updatePositionFromSpherical();
    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset, float sensitivity) {
    // 縮放 (調整距離)
    m_distance -= yOffset * sensitivity;
    m_distance = std::clamp(m_distance, m_minDistance, m_maxDistance);
    
    // 更新位置
    updatePositionFromSpherical();
    updateCameraVectors();
}

void Camera::processKeyboard(int key, float deltaTime) {
    const float speed = 5.0f * deltaTime;
    
    if (key == GLFW_KEY_W) {
        // 向前移動目標
        m_target += m_front * speed;
        updatePositionFromSpherical();
    }
    if (key == GLFW_KEY_S) {
        // 向後移動目標
        m_target -= m_front * speed;
        updatePositionFromSpherical();
    }
    if (key == GLFW_KEY_A) {
        // 向左移動目標
        m_target -= m_right * speed;
        updatePositionFromSpherical();
    }
    if (key == GLFW_KEY_D) {
        // 向右移動目標
        m_target += m_right * speed;
        updatePositionFromSpherical();
    }
    if (key == GLFW_KEY_Q) {
        // 向上移動目標
        m_target += glm::vec3(0.0f, 1.0f, 0.0f) * speed;
        updatePositionFromSpherical();
    }
    if (key == GLFW_KEY_E) {
        // 向下移動目標
        m_target -= glm::vec3(0.0f, 1.0f, 0.0f) * speed;
        updatePositionFromSpherical();
    }
    
    updateCameraVectors();
}

void Camera::reset() {
    m_position = m_initialPosition;
    m_target = m_initialTarget;
    m_up = m_initialUp;
    
    // 重新計算球座標
    glm::vec3 direction = m_position - m_target;
    m_distance = glm::length(direction);
    
    if (m_distance > 0.0f) {
        direction = glm::normalize(direction);
        m_elevation = glm::degrees(asin(direction.y));
        m_azimuth = glm::degrees(atan2(direction.x, direction.z));
    }
    
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // 計算前方向量
    m_front = glm::normalize(m_target - m_position);
    
    // 計算右方向量
    m_right = glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));
    
    // 計算上方向量
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::updatePositionFromSpherical() {
    // 將球座標轉換為笛卡爾座標
    float azimuthRad = glm::radians(m_azimuth);
    float elevationRad = glm::radians(m_elevation);
    
    float x = m_distance * cos(elevationRad) * sin(azimuthRad);
    float y = m_distance * sin(elevationRad);
    float z = m_distance * cos(elevationRad) * cos(azimuthRad);
    
    m_position = m_target + glm::vec3(x, y, z);
}

} // namespace Rendering
