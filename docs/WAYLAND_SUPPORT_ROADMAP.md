# Wayland 支持路线图

## 当前状态

### Qt 5.15 Wayland 后端问题

**版本**: Qt 5.15.13
**问题**: 已知的对话框渲染不完整

| 组件 | Wayland 后端 | X11 后端 |
|------|-------------|---------|
| 主窗口 | ✅ 正常 | ✅ 正常 |
| 节点编辑器 | ✅ 正常 | ✅ 正常 |
| QFileDialog | ❌ 渲染不完整 | ✅ 正常 |
| QMessageBox | ⚠️ 部分问题 | ✅ 正常 |
| QMenu | ⚠️ 部分问题 | ✅ 正常 |

## 解决方案

### 方案 A: 使用 X11 后端（当前方案）✅

**状态**: 已实现
**优点**:
- ✅ 立即可用
- ✅ 完全兼容
- ✅ 无性能损失
- ✅ 稳定可靠

**缺点**:
- ⚠️ 不是原生 Wayland
- ⚠️ 依赖 XWayland

**实现**:
```bash
./VisionBox.sh  # 使用 X11 后端
```

### 方案 B: 升级到 Qt 6（推荐长期方案）

**状态**: 待实施
**优点**:
- ✅ 原生 Wayland 支持
- ✅ 改进的渲染器
- ✅ 更好的性能
- ✅ 现代化的 API

**缺点**:
- ⚠️ 需要大量工作
- ⚠️ API 变化需要代码调整
- ⚠️ 需要充分测试

**工作量评估**:
- 代码迁移: 2-3 周
- 测试调试: 1-2 周
- 总计: 3-5 周

### 方案 C: 修复 Qt 5.15 Wayland 问题（不推荐）

**状态**: 不可行
**原因**:
- Qt 5.15 已进入维护模式
- Wayland 后端问题无法在应用层修复
- 需要 Qt 框架层面的修改

## Qt 6 迁移计划

### 阶段 1: 评估和准备（1 周）

```bash
# 安装 Qt 6
sudo apt install qt6-base-dev qt6-tools-dev

# 测试基本编译
qmake6  # 或 cmake -DCMAKE_PREFIX_PATH=/path/to/Qt6
```

**任务**:
- [ ] 安装 Qt 6 开发环境
- [ ] 评估 API 变化影响
- [ ] 创建迁移分支
- [ ] 列出需要修改的文件

### 阶段 2: 代码迁移（2-3 周）

**主要变化**:

| Qt 5 | Qt 6 | 变化类型 |
|------|------|---------|
| `QFileDialog` | `QFileDialog` | API 改进 |
| `QPointer` | `QPointer` | 无变化 |
| `QString` | `QString` | 轻微变化 |
| `QList` | `QList` | 实现变化 |
| 容器类 | 容器类 | 类型要求更严格 |

**需要修改的代码**:
```cpp
// Qt 5
QList<QPluginLoader*> loaders;

// Qt 6（如果需要显式类型）
QList<QPluginLoader*> loaders;  // 通常兼容
```

**任务**:
- [ ] 更新 CMakeLists.txt
- [ ] 修复编译错误
- [ ] 更新已弃用的 API
- [ ] 调整第三方库（QtNodes）

### 阶段 3: 测试和调试（1-2 周）

**测试清单**:
- [ ] 单元测试通过
- [ ] 插件加载正常
- [ ] 节点创建和连接
- [ ] 对话框显示（Wayland）
- [ ] 文件序列化
- [ ] 性能测试

**Wayland 测试**:
```bash
# 纯 Wayland 后端
QT_QPA_PLATFORM=wayland ./VisionBox

# 测试所有对话框
- File → Save As
- File → Open
- Help → About
- Plugins → Plugin Info
```

### 阶段 4: 部署和文档（1 周）

**任务**:
- [ ] 更新用户文档
- [ ] 更新构建说明
- [ ] 创建 Qt 5/6 双版本支持
- [ ] 打包和发布

## 临时方案改进

### 当前方案

使用启动脚本强制 X11 后端：
```bash
./VisionBox.sh
```

### 改进方案：自动检测

创建智能启动脚本，自动检测会话类型：

```bash
#!/bin/bash
# 智能启动脚本

# 检测会话类型
if [ "$XDG_SESSION_TYPE" = "wayland" ]; then
    # Wayland 会话：使用 X11 后端
    export QT_QPA_PLATFORM=xcb
    echo "[VisionBox] Wayland detected, using X11 backend for compatibility"
else
    echo "[VisionBox] Using default backend"
fi

# 启动应用
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
exec "$SCRIPT_DIR/VisionBox" "$@"
```

### 改进方案：用户选择

```bash
#!/bin/bash
# 带选项的启动脚本

case "$1" in
    --wayland)
        export QT_QPA_PLATFORM=wayland
        echo "[VisionBox] Using Wayland backend (experimental)"
        ;;
    --x11|)
        export QT_QPA_PLATFORM=xcb
        echo "[VisionBox] Using X11 backend"
        ;;
    *)
        # 自动检测
        if [ "$XDG_SESSION_TYPE" = "wayland" ]; then
            export QT_QPA_PLATFORM=xcb
            echo "[VisionBox] Wayland detected, using X11 backend"
        fi
        ;;
esac

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
exec "$SCRIPT_DIR/VisionBox" "$@"
```

## 推荐方案

### 短期（当前版本）

✅ **继续使用 X11 后端**

- 修改启动脚本，添加自动检测
- 在文档中说明 Wayland 兼容性
- 提供清晰的说明和故障排除

### 中期（下一个大版本）

🔄 **升级到 Qt 6**

- 计划在 VisionBox 2.0 时迁移
- 保持 Qt 5.15 兼容性一段时间
- 提供两个版本供用户选择

### 长期（未来版本）

🚀 **纯 Qt 6 + 原生 Wayland**

- 完全移除 Qt 5 支持
- 完整的原生 Wayland 支持
- 利用 Qt 6 的新特性

## 行动建议

### 立即行动

1. ✅ **改进启动脚本**
   - 添加会话类型检测
   - 添加说明信息
   - 完善文档

2. ✅ **更新 README**
   - 说明 Wayland 兼容性
   - 提供启动命令
   - 添加故障排除

### 短期计划（1-2 个月）

1. 📋 **评估 Qt 6 迁移**
   - 创建技术方案文档
   - 评估工作量
   - 确定时间表

2. 🔬 **Qt 6 实验分支**
   - 创建 `qt6` 分支
   - 尝试基本编译
   - 测试 Wayland 支持

### 中期计划（3-6 个月）

1. 🚀 **正式迁移到 Qt 6**
   - 实施 Qt 6 迁移
   - 充分测试
   - 发布 VisionBox 2.0

## 相关资源

### Qt 6 文档

- [Qt 6 发布说明](https://www.qt.io/qt6-release-notes)
- [Qt 5 到 Qt 6 迁移指南](https://doc.qt.io/qt-6/portingguide.html)
- [Qt 6 Wayland 支持](https://doc.qt.io/qt-6/linux-wayland.html)

### 示例代码

```bash
# 克隆 Qt 6 示例
git clone https://code.qt.io/qt/qt5.git
cd qt5
git checkout 6.5.0

# 测试 Wayland 应用
cd examples/widgets/mainwindows/application
qmake6
make
QT_QPA_PLATFORM=wayland ./application
```

## 总结

**当前最佳实践**:
1. ✅ 使用 `VisionBox.sh` 启动（X11 后端）
2. ✅ 在 Wayland 环境下工作完美
3. ✅ 无性能损失
4. ✅ 稳定可靠

**未来方向**:
1. 🎯 升级到 Qt 6（原生 Wayland 支持）
2. 🎯 VisionBox 2.0 计划
3. 🎯 更好的性能和功能

**现状**: 临时方案（X11 后端）已完全解决 Wayland 兼容性问题，可以正常使用。

---

**文档版本**: 1.0
**创建日期**: 2026-01-27
**最后更新**: 2026-01-27
**作者**: VisionBox Team
