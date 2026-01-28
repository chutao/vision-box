# VisionBox 启动方式

## 概述

VisionBox 现在支持**内部 Wayland 兼容性自动检测**，无需外部脚本即可在 Wayland 环境下正常使用。

## 启动方式

### 方式 1：直接运行（推荐）

✅ **自动检测并适配**

```bash
./VisionBox
```

或者在 build 目录：
```bash
cd build
./VisionBox-1.0.0
```

**行为**：
- Wayland 会话：自动使用 X11 后端（完整兼容）
- X11 会话：使用默认后端
- 显示清晰的提示信息

### 方式 2：使用启动脚本

```bash
./VisionBox.sh
```

脚本和直接运行的行为相同，提供额外的说明信息。

### 方式 3：手动指定后端

#### 使用 X11 后端（Wayland 兼容）
```bash
./VisionBox --platform xcb
```

#### 使用原生 Wayland 后端（实验性，可能有渲染问题）
```bash
./VisionBox --platform wayland
```

#### 使用环境变量
```bash
QT_QPA_PLATFORM=xcb ./VisionBox
```

## 命令行参数

```bash
./VisionBox [options]

Options:
  --platform <backend>    Force Qt platform backend (xcb, wayland)
  -p, --plugin-dir <dir>  Load plugins from directory
  -l, --list-plugins      List all loaded plugins and exit
  --no-auto-load          Disable automatic plugin loading
  -h, --help              Show help
  -v, --version           Show version
```

## 自动检测机制

VisionBox 在启动时会：

1. 检查 `QT_QPA_PLATFORM` 环境变量
   - 如果已设置，使用指定的后端
   - 如果未设置，继续检测

2. 检查命令行 `--platform` 参数
   - 如果指定，使用指定的后端
   - 如果未指定，继续检测

3. 检测会话类型
   - Wayland 会话 → 使用 X11 后端 (xcb)
   - X11 会话 → 使用默认后端

## 示例输出

### Wayland 会话（自动检测）
```
[VisionBox] Wayland session detected, using X11 backend (xcb) for compatibility
[VisionBox] See docs/WAYLAND_COMPATIBILITY.md for details
[VisionBox] Use --platform wayland to force native Wayland backend (experimental)
Loaded 29 plugins...
```

### X11 会话
```
[VisionBox] Using default Qt backend
Loaded 29 plugins...
```

### 手动指定后端
```
[VisionBox] Using requested Qt backend: "xcb"
Loaded 29 plugins...
```

## 技术细节

### 实现位置

**文件**: `src/main.cpp`

**函数**: `setupQtBackend(int argc, char* argv[])`

**调用时机**: 在 `QApplication` 创建之前

```cpp
int main(int argc, char* argv[])
{
    // 必须在 QApplication 之前设置
    setupQtBackend(argc, argv);

    QApplication app(argc, argv);
    // ...
}
```

### 环境检测

```cpp
bool isWayland = qEnvironmentVariableIsSet("WAYLAND_DISPLAY") &&
                 (qgetenv("XDG_SESSION_TYPE") == "wayland");
```

### 后端设置

```cpp
qputenv("QT_QPA_PLATFORM", "xcb");  // 设置 X11 后端
```

## 与启动脚本的对比

| 特性 | 直接运行 | 启动脚本 |
|------|---------|---------|
| Wayland 兼容 | ✅ 自动检测 | ✅ 自动检测 |
| X11 兼容 | ✅ 自动检测 | ✅ 自动检测 |
| 说明信息 | ✅ 控制台输出 | ✅ 控制台输出 |
| 可执行性 | ✅ 开箱即用 | ✅ 开箱即用 |
| 便携性 | ✅ 无需脚本 | ⚠️ 需要脚本文件 |

## 常见问题

### Q1: 为什么默认使用 X11 后端？

**A**: Qt 5.15 的 Wayland 后端有已知的对话框渲染问题。使用 X11 后端（通过 XWayland）可以确保所有功能正常工作。

### Q2: 我可以使用原生 Wayland 吗？

**A**: 可以，但可能会有渲染问题：
```bash
./VisionBox --platform wayland
```

### Q3: 如何检查当前使用的后端？

**A**: 启动时会显示，或者查看进程环境变量：
```bash
cat /proc/$(pidof VisionBox)/environ | tr '\0' '\n' | grep QT_QPA_PLATFORM
```

### Q4: 我需要安装额外依赖吗？

**A**: 不需要。XWayland 是 Wayland 合成器的标准组件。

### Q5: 性能会受影响吗？

**A**: 不会有明显影响。XWayland 是高度优化的兼容层。

## 相关文档

- [WAYLAND_COMPATIBILITY.md](WAYLAND_COMPATIBILITY.md) - Wayland 兼容性详细说明
- [WAYLAND_SUPPORT_ROADMAP.md](WAYLAND_SUPPORT_ROADMAP.md) - Qt 6 迁移路线图

## 版本历史

- **2026-01-27 v1.1**: 添加内部 Wayland 自动检测支持
- **2026-01-27 v1.0**: 初始版本，使用外部启动脚本

---

**文档版本**: 1.1
**最后更新**: 2026-01-27
**适用版本**: VisionBox 1.0.0+
