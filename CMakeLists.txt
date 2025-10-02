cmake_minimum_required(VERSION 3.16)
project(OGCClothSimulation VERSION 1.0.0 LANGUAGES C CXX)

# 設定 C++ 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 設定編譯選項
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
endif()

# 設定 macOS 特定選項
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
endif()

# 尋找依賴庫
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

# 尋找 GLFW
pkg_check_modules(GLFW REQUIRED glfw3)

# 尋找 GLM
find_package(glm QUIET)
if(NOT glm_FOUND)
    # 嘗試通過 pkg-config 尋找 GLM
    pkg_check_modules(GLM glm)
    if(NOT GLM_FOUND)
        # 手動設定 GLM 路徑
        set(GLM_INCLUDE_DIRS "")
        
        # 常見的 GLM 安裝路徑
        set(GLM_SEARCH_PATHS
            "/opt/homebrew/include"
            "/usr/local/include"
            "/usr/include"
            "/opt/local/include"
        )
        
        foreach(SEARCH_PATH ${GLM_SEARCH_PATHS})
            if(EXISTS "${SEARCH_PATH}/glm/glm.hpp")
                set(GLM_INCLUDE_DIRS ${SEARCH_PATH})
                message(STATUS "Found GLM at: ${SEARCH_PATH}")
                break()
            endif()
        endforeach()
        
        if(NOT GLM_INCLUDE_DIRS)
            message(FATAL_ERROR "GLM not found. Please install GLM: brew install glm")
        endif()
    endif()
else()
    get_target_property(GLM_INCLUDE_DIRS glm::glm INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "Found GLM via find_package")
endif()

# 嘗試尋找 Bullet Physics
pkg_check_modules(BULLET bullet)

if(NOT BULLET_FOUND)
    message(STATUS "Bullet Physics not found via pkg-config, using simplified collision detection")
    set(USE_SIMPLIFIED_COLLISION TRUE)
    add_definitions(-DUSE_SIMPLIFIED_COLLISION)
else()
    message(STATUS "Found Bullet Physics: ${BULLET_VERSION}")
endif()

# 包含目錄
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

if(BULLET_FOUND)
    include_directories(${BULLET_INCLUDE_DIRS})
endif()

# 添加 GLAD
add_library(glad STATIC
    external/glad/src/glad.c
)
target_include_directories(glad PUBLIC external/glad/include)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# 源文件
set(PHYSICS_SOURCES
    src/physics/ClothSimulation.cpp
    src/physics/OGCContactModel.cpp
    src/physics/BulletIntegration.cpp
    src/physics/Particle.cpp
)

set(RENDERING_SOURCES
    src/rendering/OpenGLRenderer.cpp
    src/rendering/Shader.cpp
    src/rendering/Camera.cpp
    src/rendering/ContactVisualizer.cpp
)

# 主要可執行文件
add_executable(OGCClothSimulation
    src/main.cpp
    ${PHYSICS_SOURCES}
    ${RENDERING_SOURCES}
)

# 連結庫
target_link_libraries(OGCClothSimulation
    OpenGL::GL
    ${GLFW_LIBRARIES}
    glad
    ${CMAKE_DL_LIBS}
)

if(BULLET_FOUND)
    target_link_libraries(OGCClothSimulation ${BULLET_LIBRARIES})
endif()

# 添加編譯器標誌
target_compile_options(OGCClothSimulation PRIVATE ${GLFW_CFLAGS_OTHER})

if(BULLET_FOUND)
    target_compile_options(OGCClothSimulation PRIVATE ${BULLET_CFLAGS_OTHER})
endif()

# macOS 特定連結
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    
    target_link_libraries(OGCClothSimulation
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()

# 編譯定義
if(USE_SIMPLIFIED_COLLISION)
    target_compile_definitions(OGCClothSimulation PRIVATE USE_SIMPLIFIED_COLLISION)
endif()

# 顯示配置信息
message(STATUS "=== OGC Cloth Simulation Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenGL Found: ${OPENGL_FOUND}")
message(STATUS "GLFW Found: ${GLFW_FOUND}")
message(STATUS "GLM Include Dirs: ${GLM_INCLUDE_DIRS}")
message(STATUS "Bullet Physics Found: ${BULLET_FOUND}")
if(BULLET_FOUND)
    message(STATUS "Bullet Version: ${BULLET_VERSION}")
    message(STATUS "Bullet Include Dirs: ${BULLET_INCLUDE_DIRS}")
    message(STATUS "Bullet Libraries: ${BULLET_LIBRARIES}")
endif()
if(USE_SIMPLIFIED_COLLISION)
    message(STATUS "Using simplified collision detection")
endif()
message(STATUS "==========================================")
