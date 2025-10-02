# 外部依賴管理

# GLAD - OpenGL 載入器
add_library(glad STATIC
    glad/src/glad.c
)
target_include_directories(glad PUBLIC
    glad/include
)

# Bullet Physics - 下載和編譯
include(FetchContent)

# 下載 Bullet Physics
FetchContent_Declare(
    bullet3
    GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
    GIT_TAG 3.25
)

# 設定 Bullet Physics 編譯選項
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(USE_GLUT OFF CACHE BOOL "" FORCE)
set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(bullet3)

# 創建 Bullet Physics 整合庫
add_library(bullet_physics INTERFACE)
target_link_libraries(bullet_physics INTERFACE
    BulletDynamics
    BulletCollision
    LinearMath
)
target_include_directories(bullet_physics INTERFACE
    ${bullet3_SOURCE_DIR}/src
)

# GLM - 數學庫 (header-only)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# 創建 GLM 介面庫
add_library(glm_interface INTERFACE)
target_include_directories(glm_interface INTERFACE
    ${glm_SOURCE_DIR}
)
