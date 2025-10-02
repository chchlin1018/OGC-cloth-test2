#!/bin/bash

# OGC Cloth Simulation Test 2 - 自動編譯腳本
# 適用於 macOS 系統

set -e  # 遇到錯誤立即退出

echo "=== OGC Cloth Simulation Test 2 編譯腳本 ==="
echo

# 顏色定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 檢查系統
echo -e "${BLUE}檢查系統環境...${NC}"
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "✓ 檢測到 macOS 系統"
    
    # 檢測架構
    ARCH=$(uname -m)
    if [[ "$ARCH" == "arm64" ]]; then
        echo "✓ Apple Silicon (M1/M2) 架構"
        HOMEBREW_PREFIX="/opt/homebrew"
    else
        echo "✓ Intel x86_64 架構"
        HOMEBREW_PREFIX="/usr/local"
    fi
else
    echo -e "${YELLOW}⚠️  非 macOS 系統，將嘗試通用編譯${NC}"
    HOMEBREW_PREFIX="/usr/local"
fi

# 檢查必需工具
echo
echo -e "${BLUE}檢查編譯工具...${NC}"

check_command() {
    if command -v $1 &> /dev/null; then
        echo "✓ $1 已安裝"
        return 0
    else
        echo -e "${RED}✗ $1 未找到${NC}"
        return 1
    fi
}

MISSING_TOOLS=0

if ! check_command cmake; then
    echo "  請安裝 CMake: brew install cmake"
    MISSING_TOOLS=1
fi

if ! check_command make; then
    echo "  請安裝 Make (通常包含在 Xcode Command Line Tools 中)"
    echo "  運行: xcode-select --install"
    MISSING_TOOLS=1
fi

if ! check_command pkg-config; then
    echo -e "${YELLOW}  建議安裝 pkg-config: brew install pkg-config${NC}"
fi

if [ $MISSING_TOOLS -eq 1 ]; then
    echo -e "${RED}❌ 缺少必需工具，請先安裝${NC}"
    exit 1
fi

# 檢查依賴庫
echo
echo -e "${BLUE}檢查依賴庫...${NC}"

check_library() {
    local lib_name=$1
    local pkg_name=$2
    local install_cmd=$3
    
    if pkg-config --exists $pkg_name 2>/dev/null; then
        echo "✓ $lib_name 已安裝"
        return 0
    elif [ -d "$HOMEBREW_PREFIX/include/$lib_name" ] || [ -d "$HOMEBREW_PREFIX/lib" ]; then
        echo "✓ $lib_name 可能已安裝 (在 $HOMEBREW_PREFIX)"
        return 0
    else
        echo -e "${YELLOW}⚠️  $lib_name 未找到${NC}"
        if [ ! -z "$install_cmd" ]; then
            echo "  建議安裝: $install_cmd"
        fi
        return 1
    fi
}

MISSING_LIBS=0

if ! check_library "GLFW" "glfw3" "brew install glfw"; then
    MISSING_LIBS=1
fi

if ! check_library "GLM" "glm" "brew install glm"; then
    MISSING_LIBS=1
fi

if ! check_library "Bullet" "bullet" "brew install bullet"; then
    MISSING_LIBS=1
fi

if [ $MISSING_LIBS -eq 1 ]; then
    echo -e "${YELLOW}⚠️  某些依賴庫未找到，但將嘗試繼續編譯${NC}"
    echo "   如果編譯失敗，請安裝上述建議的庫"
fi

# 設定編譯目錄
BUILD_DIR="build"
echo
echo -e "${BLUE}準備編譯目錄...${NC}"

if [ -d "$BUILD_DIR" ]; then
    echo "清理舊的編譯目錄..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置 CMake
echo
echo -e "${BLUE}配置 CMake...${NC}"

CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
)

# 嘗試自動檢測依賴路徑
if [ -d "$HOMEBREW_PREFIX" ]; then
    CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=$HOMEBREW_PREFIX")
    echo "使用 Homebrew 路徑: $HOMEBREW_PREFIX"
fi

# 執行 CMake 配置
echo "執行: cmake ${CMAKE_ARGS[@]} .."
if cmake "${CMAKE_ARGS[@]}" ..; then
    echo -e "${GREEN}✓ CMake 配置成功${NC}"
else
    echo -e "${RED}❌ CMake 配置失敗${NC}"
    echo
    echo "可能的解決方案:"
    echo "1. 確保所有依賴庫已正確安裝"
    echo "2. 檢查 CMakeLists.txt 中的路徑設定"
    echo "3. 嘗試手動指定庫路徑"
    exit 1
fi

# 編譯項目
echo
echo -e "${BLUE}編譯項目...${NC}"

# 獲取 CPU 核心數
if command -v sysctl &> /dev/null; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4  # 默認值
fi

echo "使用 $CORES 個並行編譯進程"

if make -j$CORES; then
    echo -e "${GREEN}✓ 編譯成功！${NC}"
else
    echo -e "${RED}❌ 編譯失敗${NC}"
    echo
    echo "可能的解決方案:"
    echo "1. 檢查編譯錯誤信息"
    echo "2. 確保所有依賴庫版本兼容"
    echo "3. 嘗試單線程編譯: make"
    exit 1
fi

# 檢查編譯結果
echo
echo -e "${BLUE}檢查編譯結果...${NC}"

EXECUTABLE="OGCClothSimulation"

if [ -f "$EXECUTABLE" ]; then
    echo -e "${GREEN}✓ $EXECUTABLE 編譯成功${NC}"
    
    # 檢查可執行文件信息
    echo
    echo "可執行文件信息:"
    ls -lh "$EXECUTABLE"
    
    if command -v file &> /dev/null; then
        file "$EXECUTABLE"
    fi
    
    echo
    echo -e "${GREEN}🎉 編譯完成！${NC}"
    echo
    echo "運行程序:"
    echo "  cd build"
    echo "  ./$EXECUTABLE"
    echo
    echo "控制說明:"
    echo "  滑鼠左鍵拖拽: 旋轉相機"
    echo "  滑鼠滾輪:     縮放"
    echo "  WASD:         移動相機"
    echo "  空格鍵:       暫停/繼續"
    echo "  R:            重置場景"
    echo "  W:            切換線框模式"
    echo "  P:            切換粒子顯示"
    echo "  C:            切換接觸點顯示"
    echo "  ESC:          退出程序"
    
else
    echo -e "${RED}❌ 可執行文件未找到${NC}"
    echo "編譯可能未完全成功，請檢查錯誤信息"
    exit 1
fi

echo
echo -e "${BLUE}編譯腳本執行完成${NC}"
