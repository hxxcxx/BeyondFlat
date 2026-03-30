# BeyondFlat

一个 CAGD（计算机辅助几何设计）学习项目，专注于曲线和曲面的数学实现与可视化。

## 📋 项目简介

本项目旨在通过实践学习 CAGD 的核心概念，包括：

- ✅ **贝塞尔曲线（Bezier Curves）** - 已实现
- ✅ **B 样条曲线（B-Spline Curves）** - 已实现
- 🚧 **NURBS 曲线** - 计划中
- ✅ **曲面（Surfaces）** - 部分实现

## 🎯 主要特性

### 已实现功能

- **贝塞尔曲线（2D/3D）**
  - Bernstein 多项式求值
  - de Casteljau 算法
  - 导数计算
  - 交互式控制点编辑
  - 实时可视化

- **B 样条曲线（2D）**
  - de Boor 算法求值
  - 导数计算
  - 均匀节点向量
  - 交互式控制点编辑

- **贝塞尔曲面（3D）**
  - 张量积 Bezier 曲面
  - de Casteljau 算法
  - 偏导数与法向量计算
  - 网格生成与渲染

- **可视化系统**
  - 基于 ImGui 的现代 UI
  - 交互式画布（2D/3D）
  - 控制点拖拽
  - 切线/法向量可视化
  - 参数调节滑块

- **3D 渲染引擎**
  - OpenGL 实现3D曲面渲染
  - OrbitCamera 轨道相机控制
  - 网格/线框/点渲染
  - 坐标轴与网格绘制

- **数学库**
  - 基于 Eigen 的线性代数支持
  - 2D/3D 向量运算
  - 坐标变换
  - OpenGL 数学辅助函数

### 交互功能

- 🖱️ **左键点击空白处** - 添加控制点
- 🖱️ **左键拖动控制点** - 移动控制点
- 🖱️ **右键点击控制点** - 删除控制点
- 🎛️ **参数滑块** - 调节切线、节点和曲线上的点
- ⌨️ **键盘快捷键** - 节点插入等操作

## 📁 项目结构

```
BeyondFlat/
├── src/                          # 数学库核心代码
│   ├── bezier/                   # 贝塞尔曲线/曲面实现
│   │   ├── bezier_curve.cc/h
│   │   └── bezier_surface.cc/h
│   ├── bspline/                  # B 样条曲线实现
│   │   └── bspline_curve.cc/h
│   ├── nurbs/                    # NURBS（计划中）
│   └── common/                   # 通用工具
│       ├── common.cc/h
│       └── glmath.cc/h           # OpenGL 数学库
│
├── viewer/                       # 可视化应用
│   ├── main.cc                   # 程序入口
│   ├── framework/                # 框架核心
│   │   ├── application.cc/h      # 主应用程序
│   │   ├── gl/                   # OpenGL 相关
│   │   │   ├── camera.cc/h
│   │   │   ├── renderer.cc/h
│   │   │   └── viewport.cc/h
│   │   └── base/
│   │       └── curve_editor.h    # 曲线编辑器基类
│   └── impl/                     # 具体实现
│       ├── bezier/               # 贝塞尔曲线/曲面编辑器
│       │   ├── curve_editor.cc/h
│       │   └── surface_editor.cc/h
│       └── bspline/              # B 样条曲线编辑器
│           └── curve_editor.cc/h
│
├── .vscode/                      # VS Code 配置
│   ├── launch.json               # 调试配置
│   ├── tasks.json                # 构建任务
│   └── c_cpp_properties.json     # IntelliSense 配置
│
└── build/                        # 构建输出目录
    ├── bin/                      # 可执行文件
    └── lib/                      # 库文件
```

## 🚀 快速开始

### 环境要求

- **编译器**: Visual Studio 2022 (MSVC 19.44+)
- **CMake**: 3.15+
- **vcpkg**: 包管理器
- **操作系统**: Windows 10/11

### 依赖管理

本项目使用 [vcpkg](https://github.com/microsoft/vcpkg) 管理依赖：

```powershell
# vcpkg 安装位置
F:/APP/vcpkg

# 已安装的包
- eigen3:x64-windows
- glfw3:x64-windows
- imgui:x64-windows
```

### 构建步骤

#### 方法一：使用 PowerShell 脚本

```powershell
# Release 版本
.\build_release.ps1

# RelWithDebInfo 版本（推荐用于调试）
.\build_relwithdebinfo.ps1
```

#### 方法二：手动构建

```powershell
# 创建构建目录
mkdir build
cd build

# 配置 CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=F:/APP/vcpkg/scripts/buildsystems/vcpkg.cmake

# 构建
cmake --build . --config Release
```

### 运行程序

```powershell
# Release 版本
.\build\bin\Release\cagd_viewer.exe

# RelWithDebInfo 版本
.\build\bin\RelWithDebInfo\cagd_viewer.exe
```

## 🎮 使用说明

### 界面布局

```
┌─────────────────────────────────────────────────────────┐
│  ┌─────────────┐  ┌──────────────────────────────────┐  │
│  │  曲线选择器  │  │                                  │  │
│  │  (350x200)  │  │        Canvas 画布               │  │
│  │             │  │                                  │  │
│  ├─────────────┤  │                                  │  │
│  │  控制面板    │  │      交互式曲线编辑              │  │
│  │  (350x520)  │  │                                  │  │
│  │             │  │                                  │  │
│  └─────────────┘  └──────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### 控制面板选项

#### 曲线编辑器
- **Show Control Points** - 显示控制点
- **Show Control Polygon** - 显示控制多边形
- **Show Tangent** - 显示切线
- **Show Point on Curve** - 显示曲线上的点
- **Show Knot Points** (B样条) - 显示节点向量对应的点

#### 曲面编辑器
- **Show Surface** - 显示曲面
- **Show Control Points** - 显示控制点网格
- **Show Axes** - 显示坐标轴
- **Show Grid** - 显示网格
- **U/V Resolution** - 曲面网格分辨率

### 鼠标操作

| 操作 | 功能 |
|------|------|
| 左键点击空白处 | 添加控制点 |
| 左键拖动控制点 | 移动控制点 |
| 右键点击控制点 | 删除控制点 |

## 📚 技术栈

- **语言**: C++17
- **构建系统**: CMake
- **包管理**: vcpkg
- **数学库**: Eigen 5.0.1
- **窗口管理**: GLFW 3.4
- **GUI 框架**: ImGui 1.92.6
- **渲染**: OpenGL + ImGui DrawList

## 📖 学习资料

### NURBS 详解

- [B站视频：NURBS详解](https://www.bilibili.com/video/BV1WN4y117aj/?share_source=copy_web&vd_source=6ebb0754dc3f18fc0e61fca680a38c61) - B样条与NURBS学习资料

## 🗺️ 开发路线图

### Phase 1: 基础曲线 ✅
- [x] 贝塞尔曲线（2D/3D）
- [x] 交互式编辑
- [x] 可视化系统

### Phase 2: 高级曲线 ✅
- [x] B 样条曲线（2D）
- [ ] NURBS 曲线
- [ ] 曲线插值与拟合

### Phase 3: 曲面（进行中）
- [x] 贝塞尔曲面
- [ ] B 样条曲面
- [ ] NURBS 曲面
- [ ] 曲面修剪与拼接

### Phase 4: 高级功能（未来）
- [ ] 曲线/曲面求交
- [ ] 布尔运算
- [ ] 实体建模

## 📄 许可证

本项目仅用于学习目的。详见 [LICENSE](LICENSE) 文件。

## 🔗 相关链接

- [Eigen](https://eigen.tuxfamily.org/) - 强大的 C++ 模板库
- [GLFW](https://www.glfw.org/) - OpenGL 窗口管理库
- [ImGui](https://github.com/ocornut/imgui) - 即时模式 GUI 库
- [vcpkg](https://github.com/microsoft/vcpkg) - C++ 包管理器

---

