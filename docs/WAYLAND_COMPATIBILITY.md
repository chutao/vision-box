# Wayland 兼容性问题及解决方案

## 问题描述

在 Wayland 显示服务器环境下运行 VisionBox 时，可能会遇到以下问题：

- ❌ 对话框显示不完整
- ❌ 部分控件无法正常渲染
- ❌ 底部按钮不可见
- ❌ 文件对话框被截断

## 根本原因

**Qt 5.15 Wayland 后端兼容性问题**

Qt 5.15 虽然提供了 Wayland 后端支持，但存在已知的渲染问题，特别是：
- QFileDialog 对话框尺寸计算不准确
- 某些复杂控件在 Wayland 合成器中渲染不完整
- 窗口管理器集成问题

## 环境检测

检查当前会话类型：

```bash
echo "XDG_SESSION_TYPE: $XDG_SESSION_TYPE"
echo "WAYLAND_DISPLAY: $WAYLAND_DISPLAY"
echo "DISPLAY: $DISPLAY"
```

**输出示例**（Wayland 会话）：
```
XDG_SESSION_TYPE: wayland
WAYLAND_DISPLAY: wayland-1
DISPLAY: :1
```

## 解决方案

### 方案 1：使用 X11 后端（推荐）

VisionBox 提供了启动脚本 `VisionBox.sh`，自动强制使用 X11 后端：

```bash
./VisionBox.sh
```

**脚本内容**：
```bash
#!/bin/bash
# 强制使用 X11 后端（通过 XWayland）
export QT_QPA_PLATFORM=xcb

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 启动 VisionBox
exec "$SCRIPT_DIR/VisionBox" "$@"
```

### 方案 2：手动设置环境变量

如果不使用启动脚本，可以直接设置环境变量：

```bash
QT_QPA_PLATFORM=xcb ./VisionBox
```

### 方案 3：全局设置（所有 Qt 应用）

如果希望所有 Qt 应用都使用 X11 后端，可以设置环境变量：

**临时设置**（当前会话）：
```bash
export QT_QPA_PLATFORM=xcb
```

**永久设置**（添加到 `~/.bashrc` 或 `~/.profile`）：
```bash
echo 'export QT_QPA_PLATFORM=xcb' >> ~/.bashrc
source ~/.bashrc
```

## 技术细节

### Qt 平台插件 (Platform Plugin)

Qt 支持多个平台插件：

| 平台插件 | 说明 | Wayland 兼容性 |
|---------|------|---------------|
| `wayland` | 原生 Wayland 后端 | ⚠️ 有渲染问题 |
| `xcb` | X11 后端（通过 XWayland） | ✅ 完全兼容 |
| `wayland-egl` | Wayland with EGL | ⚠️ 实验性 |

### XWayland 兼容层

XWayland 是 Wayland 中运行的 X11 兼容层：

```
┌─────────────────────────────┐
│     Wayland Compositor      │
│  (GNOME Shell, KDE Plasma)  │
└─────────────────────────────┘
           │
           ├─ Wayland native apps
           │
           └─ XWayland
                │
                └─ X11 apps (Qt with xcb)
```

使用 X11 后端的优势：
- ✅ 完全的 Qt 5.15 功能支持
- ✅ 所有对话框正常渲染
- ✅ 稳定性和兼容性最好
- ✅ 自动集成到 Wayland 会话

### 性能影响

使用 X11 后端（通过 XWayland）的性能影响：

| 方面 | 影响 | 说明 |
|------|------|------|
| CPU 使用 | 微增 (~1-2%) | XWayland 转换开销 |
| 内存使用 | 微增 (~5-10MB) | X11 库加载 |
| 渲染性能 | 无明显差异 | 现代硬件上无感知 |
| 响应速度 | 无明显差异 | XWayland 优化良好 |

## 验证解决方案

### 1. 检查 Qt 平台

```bash
# 查看进程环境变量
cat /proc/$(pidof VisionBox)/environ | tr '\0' '\n' | grep QT_QPA_PLATFORM
```

**预期输出**：
```
QT_QPA_PLATFORM=xcb
```

### 2. 测试对话框

启动应用后测试：
- File → Save As（保存对话框）
- File → Open（打开对话框）
- Plugins → Plugin Info（插件信息对话框）

**预期结果**：所有对话框完整显示，底部按钮可见。

## 常见问题

### Q1: 使用 X11 后端会影响性能吗？

**A**: 不会有明显影响。XWayland 是高度优化的兼容层，在现代硬件上性能差异可以忽略。

### Q2: 未来会支持原生 Wayland 吗？

**A**: Qt 6 对 Wayland 的支持大幅改进。未来升级到 Qt 6 后，可以尝试使用原生 Wayland 后端。

### Q3: 是否可以在运行时切换平台后端？

**A**: 不可以。平台后端必须在应用启动前通过环境变量设置。

### Q4: 启动脚本是必须的吗？

**A**: 不是必须的，但强烈推荐使用。它简化了启动命令并确保正确的配置。

### Q5: 如何确认当前使用的后端？

**A**:
```bash
# 方法1：检查环境变量
echo $QT_QPA_PLATFORM

# 方法2：查看运行中的应用
ps aux | grep VisionBox
cat /proc/<PID>/environ | tr '\0' '\n' | grep QT_QPA_PLATFORM
```

## 相关资源

### Qt 文档

- [Qt Platform Plugins](https://doc.qt.io/qt-5/qt-platforms.html)
- [Wayland in Qt 5](https://wiki.qt.io/Wayland)
- [Qt 6 Wayland Improvements](https://www.qt.io/blog/wayland-in-qt-6)

### Wayland 相关

- [Wayland 官方网站](https://wayland.freedesktop.org/)
- [XWayland 文档](https://wayland.freedesktop.org/xserver.html)

## 版本历史

- **2026-01-27**: 初始版本，记录 Qt 5.15 Wayland 兼容性问题及解决方案
- **待定**: Qt 6 迁移后更新文档

## 总结

**推荐做法**：
1. ✅ 在 Wayland 会话下使用 `VisionBox.sh` 启动脚本
2. ✅ 脚本自动设置 `QT_QPA_PLATFORM=xcb`
3. ✅ 所有对话框和控件正常显示
4. ✅ 无性能损失
5. ✅ 完美兼容 Wayland 环境

**不推荐**：
- ❌ 直接使用 `./VisionBox`（会使用 Wayland 后端，有渲染问题）
- ❌ 尝试使用 `QT_QPA_PLATFORM=wayland`（已知问题）

---

**文档版本**: 1.0
**最后更新**: 2026-01-27
**适用版本**: VisionBox 1.0.0, Qt 5.15.13
