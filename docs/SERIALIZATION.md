# 流程序列化功能文档

## 概述

VisionBox 支持完整的节点图序列化功能，可以保存和加载节点流程，包括所有节点、连接和参数。

## 功能特性

- ✅ **保存节点图**: 将当前工作流保存到 .vbjson 文件
- ✅ **加载节点图**: 从文件加载完整的工作流
- ✅ **修改跟踪**: 自动检测图的修改状态
- ✅ **未保存提示**: 关闭时提示保存未保存的更改
- ✅ **完整序列化**: 保存节点类型、位置、参数和连接关系

## 文件格式

VisionBox 使用 **.vbjson** (VisionBox JSON) 文件格式，基于标准的 JSON 格式，便于阅读和调试。

### 文件结构示例

```json
{
  "nodes": [
    {
      "id": 0,
      "model_name": "ImageLoaderModel",
      "position": { "x": 100, "y": 100 },
      "data": {
        "caption": "Image Loader",
        "parameters": {
          "fileName": "/path/to/image.jpg"
        }
      }
    },
    {
      "id": 1,
      "model_name": "BlurModel",
      "position": { "x": 400, "y": 100 },
      "data": {
        "caption": "Blur",
        "parameters": {
          "kernelSize": 5
        }
      }
    }
  ],
  "connections": [
    {
      "id": 0,
      "in_id": 1,
      "in_index": 0,
      "out_id": 0,
      "out_index": 0
    }
  ]
}
```

## 使用方法

### 保存工作流

1. 创建或编辑节点图
2. 选择 **File → Save** 或 **File → Save As**
3. 选择保存位置和文件名
4. 点击保存

**自动添加扩展名**: 如果文件名没有 `.vbjson` 后缀，系统会自动添加。

**快捷键**: `Ctrl+S` (Save), `Ctrl+Shift+S` (Save As)

### 打开工作流

1. 选择 **File → Open**
2. 浏览并选择 .vbjson 文件
3. 点击打开

**快捷键**: `Ctrl+O`

### 新建工作流

1. 选择 **File → New**
2. 如果当前图有未保存的更改，会提示保存
3. 创建新的空白画布

**快捷键**: `Ctrl+N`

## 修改状态

VisionBox 会自动跟踪工作流的修改状态：

- **未修改**: 窗口标题显示 "VisionBox - 文件名"
- **已修改**: 窗口标题显示 "VisionBox - 文件名 *" (星号表示未保存的更改)

### 触发修改的操作

以下操作会自动标记工作流为已修改：

- 添加节点
- 删除节点
- 移动节点
- 创建连接
- 删除连接
- 修改节点参数

## 自动保存提示

关闭窗口或创建新图时，如果有未保存的更改，VisionBox 会弹出对话框：

```
The document has been modified.
Do you want to save your changes?

[Save] [Discard] [Cancel]
```

- **Save**: 保存更改并继续操作
- **Discard**: 放弃更改并继续操作
- **Cancel**: 取消操作

## 技术实现

### 核心组件

1. **DataFlowGraphModel**: 图模型序列化
   - `save()`: 序列化节点图到 JSON
   - `load()`: 从 JSON 反序列化节点图

2. **MainWindow**: 用户界面和文件操作
   - `onSave()`: 保存处理
   - `onOpen()`: 打开文件处理
   - `setModified()`: 修改状态管理

3. **QtNodes 基类**: 序列化基础设施
   - 节点序列化
   - 连接序列化
   - 参数序列化

### 修改跟踪机制

使用 Qt 信号-槽机制跟踪图变更：

```cpp
// 连接图模型信号到修改状态
connect(m_graphModel, &QtNodes::AbstractGraphModel::nodeCreated,
        this, [this]() { setModified(true); });
connect(m_graphModel, &QtNodes::AbstractGraphModel::nodeDeleted,
        this, [this]() { setModified(true); });
connect(m_graphModel, &QtNodes::AbstractGraphModel::connectionCreated,
        this, [this]() { setModified(true); });
connect(m_graphModel, &QtNodes::AbstractGraphModel::connectionDeleted,
        this, [this]() { setModified(true); });
connect(m_graphModel, &QtNodes::AbstractGraphModel::nodePositionUpdated,
        this, [this]() { setModified(true); });
```

## 错误处理

### 保存失败

如果保存文件失败，会显示错误对话框：

```
Error
Could not save file: /path/to/file.vbjson
```

**常见原因**:
- 磁盘空间不足
- 文件权限不足
- 路径不存在

### 加载失败

如果加载文件失败，会显示错误对话框：

```
Error
Could not open file: /path/to/file.vbjson
```

**常见原因**:
- 文件不存在
- 文件格式错误
- 文件权限不足
- 缺少必需的插件

## 最佳实践

1. **定期保存**: 在进行重要更改后及时保存
2. **版本控制**: 使用 Git 等工具管理 .vbjson 文件
3. **备份**: 定期备份重要的工作流文件
4. **命名规范**: 使用有意义的文件名，如 `edge-detection-pipeline`
   - 注意：扩展名 `.vbjson` 会自动添加，无需手动输入
5. **文档**: 为复杂工作流创建 README 文件说明

## 未来增强

计划中的功能：

- [ ] 自动保存（可配置间隔）
- [ ] 自动备份（保存时创建备份）
- [ ] 历史记录（撤销/ redo 超出会话）
- [ ] 导出为图片（工作流截图）
- [ ] 导出为 Python 脚本
- [ ] 模板系统（常用工作流模板）

## 相关文件

- `src/ui/DataFlowGraphModel.cpp`: 序列化实现
- `src/ui/MainWindow.cpp`: UI 处理
- `docs/SERIALIZATION.md`: 本文档

## 示例工作流

示例工作流文件位于：`examples/workflows/`

- `edge-detection.vbjson`: 边缘检测流程
- `face-detection.vbjson`: 人脸检测流程
- `color-tracking.vbjson`: 颜色跟踪流程

## 故障排除

### 问题：加载后节点缺失

**原因**: 缺少必需的插件

**解决**:
1. 检查已加载插件列表 (Plugins → Plugin Info)
2. 确保所有节点对应的插件已加载
3. 重新加载缺失的插件

### 问题：修改状态不更新

**原因**: 信号连接失败

**解决**:
1. 重启应用程序
2. 检查控制台是否有错误消息
3. 如果问题持续，报告 Bug

### 问题：文件很大

**原因**: 节点图包含大量图像数据

**解决**:
- 图像数据应该通过路径引用，而不是直接嵌入
- 使用相对路径提高可移植性
- 避免在节点参数中存储大型二进制数据

---

**文档版本**: 1.0
**最后更新**: 2026-01-27
**作者**: VisionBox Team
