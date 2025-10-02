# OGC Cloth Simulation Test 2

一個基於 **OGC (Offset Geometric Contact) Contact Model** 和 **Bullet Physics** 的高級布料物理模擬項目，使用 **OpenGL** 進行實時 3D 可視化，不依賴 Qt 框架，專為 macOS 優化。

## 🎯 項目特點

### 物理模擬
- **OGC Contact Model**: 先進的偏移幾何接觸算法，提供精確的碰撞檢測
- **Bullet Physics 整合**: 高效的碰撞檢測和剛體物理
- **Verlet 積分**: 數值穩定的粒子更新方法
- **多層約束系統**: 結構、剪切和彎曲約束的完美結合

### 視覺效果
- **實時 3D 渲染**: 使用 OpenGL 3.3 Core Profile
- **接觸點可視化**: 顯示接觸點位置和大小
- **接觸力可視化**: 顯示接觸力的方向和強度
- **接觸法線顯示**: 可視化接觸表面法線
- **OGC 偏移幾何**: 顯示 OGC 模型的偏移幾何結構

### 交互功能
- **相機控制**: 滑鼠拖拽旋轉，滾輪縮放，鍵盤移動
- **實時參數調整**: 重力、風力、阻尼等物理參數
- **多種顯示模式**: 線框、粒子、實體渲染切換
- **場景控制**: 暫停、重置、單步執行

## 🏗️ 項目架構

```
OGC-cloth-test2/
├── src/
│   ├── physics/           # 物理模擬核心
│   │   ├── ClothSimulation.cpp    # 布料模擬主類
│   │   ├── OGCContactModel.cpp    # OGC 接觸模型
│   │   ├── BulletIntegration.cpp  # Bullet Physics 整合
│   │   └── Particle.cpp           # 粒子系統
│   ├── rendering/         # OpenGL 渲染系統
│   │   ├── OpenGLRenderer.cpp     # 主渲染器
│   │   ├── ContactVisualizer.cpp  # 接觸可視化
│   │   ├── Shader.cpp             # 著色器管理
│   │   └── Camera.cpp             # 相機控制
│   └── main.cpp           # 主程序入口
├── include/               # 頭文件
├── external/              # 外部依賴
│   ├── glad/              # OpenGL 載入器
│   └── glm/               # 數學庫
├── shaders/               # GLSL 著色器
├── docs/                  # 文檔
└── CMakeLists.txt         # CMake 配置
```

## 🚀 快速開始

### 系統要求
- **macOS 10.14+** (支援 Intel 和 Apple Silicon)
- **Xcode Command Line Tools** 或完整 Xcode
- **CMake 3.16+**
- **OpenGL 3.3+** 支援

### 依賴安裝

#### 使用 Homebrew (推薦)
```bash
# 安裝基本依賴
brew install cmake glfw glm bullet

# 安裝 pkg-config (可選，用於依賴檢測)
brew install pkg-config
```

#### 手動安裝
1. **GLFW**: 從 [官網](https://www.glfw.org/) 下載並安裝
2. **GLM**: 從 [GitHub](https://github.com/g-truc/glm) 下載頭文件庫
3. **Bullet Physics**: 從 [官網](https://pybullet.org/wordpress/) 下載並編譯

### 編譯和運行

```bash
# 克隆項目
git clone https://github.com/chchlin1018/OGC-cloth-test2.git
cd OGC-cloth-test2

# 創建編譯目錄
mkdir build && cd build

# 配置項目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 編譯
make -j$(sysctl -n hw.ncpu)

# 運行
./OGCClothSimulation
```

## 🎮 使用說明

### 控制方式

#### 相機控制
- **滑鼠左鍵拖拽**: 旋轉相機視角
- **滑鼠滾輪**: 縮放視圖
- **WASD**: 移動相機位置
- **QE**: 垂直移動相機

#### 模擬控制
- **空格鍵**: 暫停/繼續模擬
- **R**: 重置場景到初始狀態
- **ESC**: 退出程序

#### 顯示選項
- **W**: 切換線框模式 (顯示布料約束)
- **P**: 切換粒子顯示
- **C**: 切換接觸點和接觸力顯示
- **N**: 切換接觸法線顯示
- **O**: 切換 OGC 偏移幾何顯示

### 場景說明

程序展示以下物理過程：

1. **初始狀態**: 20×20 的布料網格懸掛在空中 (高度 3.0m)
2. **自由落體**: 布料在重力作用下開始下落
3. **圓柱碰撞**: 布料與圓柱體發生碰撞，展現 OGC 接觸模型的精確性
4. **形變和滑落**: 布料在圓柱體上發生形變，然後滑落
5. **地面接觸**: 布料最終落到地面，形成自然的褶皺

## 🔬 技術細節

### OGC Contact Model

OGC (Offset Geometric Contact) 模型是一種先進的接觸檢測和解析算法：

- **偏移幾何**: 為每個物體創建偏移表面，提高接觸檢測精度
- **正交接觸力**: 確保接觸力垂直於接觸表面
- **無穿透保證**: 數學上保證物體不會相互穿透
- **穩定性**: 提供比傳統方法更穩定的數值解

### Bullet Physics 整合

- **碰撞檢測**: 使用 Bullet 的高效碰撞檢測算法
- **廣相檢測**: 快速篩選潛在碰撞對
- **窄相檢測**: 精確計算接觸點和法線
- **形狀支援**: 支援球體、盒子、圓柱體、網格等多種形狀

### 布料物理

- **粒子系統**: 基於質點-彈簧模型
- **Verlet 積分**: 保持數值穩定性的積分方法
- **約束求解**: 使用 PBCS (Position Based Constraint Solving)
- **多層約束**: 結構、剪切、彎曲約束的層次化處理

## 📊 性能特點

- **實時模擬**: 60 FPS 流暢運行
- **高精度**: OGC 模型提供亞毫米級精度
- **可擴展**: 支援數千個粒子的大規模模擬
- **內存效率**: 優化的數據結構和算法

## 🛠️ 開發和調試

### 編譯選項

```bash
# Debug 模式 (包含調試信息)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release 模式 (性能優化)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 啟用詳細輸出
cmake .. -DVERBOSE_OUTPUT=ON
```

### 調試功能

- **接觸點可視化**: 紅色球體表示接觸點
- **接觸力向量**: 彩色線段表示力的方向和大小
- **接觸法線**: 藍色線段表示表面法線
- **性能統計**: 實時顯示 FPS 和計算時間

## 📚 API 文檔

### 主要類別

#### `Physics::ClothSimulation`
布料模擬的主控制類，負責整個物理過程。

```cpp
// 初始化布料
bool initialize(int width, int height, 
               const glm::vec2& clothSize,
               const glm::vec3& position);

// 更新模擬
void update(float deltaTime);

// 添加碰撞體
void addCylinder(const glm::vec3& center, float radius, float height);
void addFloor(const glm::vec3& center, const glm::vec3& size);
```

#### `Physics::OGCContactModel`
OGC 接觸模型的核心實現。

```cpp
// 處理接觸列表
void processContacts(std::vector<OGCContact>& contacts, float deltaTime);

// 計算接觸力
glm::vec3 calculateContactForce(const OGCContact& contact);
```

#### `Rendering::OpenGLRenderer`
OpenGL 渲染系統的主類。

```cpp
// 渲染布料
void renderClothParticles(const std::vector<Particle*>& particles);
void renderClothConstraints(const std::vector<Particle*>& particles,
                           const std::vector<std::pair<int, int>>& constraints);

// 渲染接觸
void renderContacts(const std::vector<OGCContact>& contacts);
```

## 🤝 貢獻指南

歡迎貢獻代碼、報告問題或提出改進建議！

### 開發環境設置

1. Fork 本項目
2. 創建功能分支: `git checkout -b feature/amazing-feature`
3. 提交更改: `git commit -m 'Add amazing feature'`
4. 推送分支: `git push origin feature/amazing-feature`
5. 創建 Pull Request

### 代碼規範

- 使用 C++17 標準
- 遵循 Google C++ Style Guide
- 添加適當的註釋和文檔
- 包含單元測試

## 📄 許可證

本項目採用 MIT 許可證 - 詳見 [LICENSE](LICENSE) 文件。

## 🙏 致謝

- **OGC Contact Model**: 基於相關學術研究
- **Bullet Physics**: 優秀的物理引擎
- **GLFW**: 跨平台 OpenGL 框架
- **GLM**: OpenGL 數學庫
- **GLAD**: OpenGL 載入器

## 📞 聯繫方式

如有問題或建議，請通過以下方式聯繫：

- **GitHub Issues**: [項目問題頁面](https://github.com/chchlin1018/OGC-cloth-test2/issues)
- **Email**: 項目維護者郵箱

---

**OGC Cloth Simulation Test 2** - 展示先進物理模擬技術的力量 🚀
