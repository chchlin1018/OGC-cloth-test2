cmake_minimum_required(VERSION 3.16)
project(OGC-cloth-test2 VERSION 1.0.0 LANGUAGES CXX)

# 設定 C++ 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS 特定設定
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    # 支援 Intel 和 Apple Silicon
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for OS X")
endif()

# 編譯選項
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 尋找必要的套件
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# 包含目錄
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# 外部依賴
add_subdirectory(external)

# 源文件
set(PHYSICS_SOURCES
    src/physics/ClothSimulation.cpp
    src/physics/OGCContactModel.cpp
    src/physics/BulletIntegration.cpp
    src/physics/Particle.cpp
    src/physics/Constraint.cpp
)

set(RENDERING_SOURCES
    src/rendering/OpenGLRenderer.cpp
    src/rendering/Shader.cpp
    src/rendering/Camera.cpp
    src/rendering/ContactVisualizer.cpp
)

set(UTILS_SOURCES
    src/utils/Math.cpp
    src/utils/Timer.cpp
    src/utils/Logger.cpp
)

# 主要可執行文件
add_executable(cloth_simulation
    src/main.cpp
    ${PHYSICS_SOURCES}
    ${RENDERING_SOURCES}
    ${UTILS_SOURCES}
)

# 連結庫
target_link_libraries(cloth_simulation
    OpenGL::GL
    glfw
    glad
    bullet_physics
    ${CMAKE_DL_LIBS}
)

# macOS 特定設定
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(cloth_simulation
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()

# 範例程序
add_subdirectory(examples)

# 安裝規則
install(TARGETS cloth_simulation
    RUNTIME DESTINATION bin
)
