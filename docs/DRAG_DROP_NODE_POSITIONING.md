# 拖拽添加节点位置优化

## 问题描述

之前从算子工具箱拖拽节点到画布时，节点总是出现在屏幕中间位置，而不是鼠标拖拽释放的位置，用户体验不佳。

## 解决方案

### 问题根源

第一次拖拽时，节点的图形对象（NodeGraphicsObject）还没有完全初始化，导致 `setNodeData()` 调用失败。第二次及以后，场景已经初始化完成，所以能正常工作。

### 实现细节

使用 `QTimer::singleShot(0, ...)` 延迟设置位置到下一个事件循环，确保节点完全初始化：

```cpp
void VisionBoxGraphicsView::createNodeFromDrag(const QString& modelName, const QPoint& viewPos)
{
    auto* scene = dynamic_cast<::QtNodes::DataFlowGraphicsScene*>(this->scene());
    auto& graphModel = scene->graphModel();

    // 创建节点
    ::QtNodes::NodeId nodeId = graphModel.addNode(modelName);

    // 转换坐标
    QPointF scenePos = mapToScene(viewPos);
    QPointF adjustedPos = scenePos - QPointF(80, 40);

    // 延迟到下一个事件循环设置位置
    // 确保节点的图形对象已完全初始化
    QTimer::singleShot(0, this, [this, nodeId, adjustedPos]() {
        setNodePositionDelayed(nodeId, adjustedPos);
    });
}

void VisionBoxGraphicsView::setNodePositionDelayed(::QtNodes::NodeId nodeId, const QPointF& pos)
{
    auto* scene = dynamic_cast<::QtNodes::DataFlowGraphicsScene*>(this->scene());
    auto& graphModel = scene->graphModel();

    // 设置节点位置
    bool success = graphModel.setNodeData(nodeId, ::QtNodes::NodeRole::Position, QVariant::fromValue(pos));

    if (success) {
        qDebug() << "Successfully set position for node" << nodeId << "to" << pos;
    } else {
        qWarning() << "Failed to set position for node" << nodeId;
    }
}
```

### 关键 API

- **`mapToScene(viewPos)`**: 将视图（QWidget）坐标转换为场景（QGraphicsScene）坐标
- **`NodeRole::Position`**: QtNodes 枚举，表示节点位置角色
- **`setNodeData()`**: 设置节点属性，包括位置、大小、标题等
- **`QTimer::singleShot(0, ...)`**: 延迟到下一个事件循环执行，确保对象已初始化

### 延迟初始化原理

Qt 的事件循环机制：
1. **当前事件循环**：创建节点，但图形对象未完全初始化
2. **下一个事件循环**：所有图形对象已创建，可以安全设置位置

使用 `QTimer::singleShot(0, ...)` 的好处：
- 0 毫秒延迟，不阻塞 UI
- 在下一个事件循环执行
- 确保节点完全初始化后设置位置
- 对用户无感知，体验流畅

代码中将节点位置偏移了 `QPointF(80, 40)`，原因：
- 典型的 QtNodes 节点宽度约为 160-200 像素
- 偏移 (80, 40) 使节点中心大致对齐鼠标位置
- 提供更好的视觉反馈和用户体验

## 效果对比

### 优化前
- 节点总是出现在屏幕中心
- 需要手动拖拽节点到目标位置
- 操作步骤多，效率低
- **第一次拖拽位置不正确**（未使用延迟设置）

### 优化后
- 节点直接出现在拖拽释放位置
- 减少了后续手动调整
- 操作直观，用户体验好
- **每次拖拽都能正确定位**（使用事件循环延迟）

## 使用说明

1. 打开 VisionBox 应用
2. 从左侧算子工具箱选择任意算子
3. 按住鼠标左键拖拽算子到画布
4. 释放鼠标，节点将出现在鼠标位置附近

## 技术细节

### 事件循环与异步设置

Qt 的事件驱动模型要求在某些情况下使用异步操作：

```cpp
// 同步调用（第一次失败）
graphModel.setNodeData(nodeId, NodeRole::Position, pos);

// 异步调用（每次成功）
QTimer::singleShot(0, this, [this, nodeId, pos]() {
    graphModel.setNodeData(nodeId, NodeRole::Position, pos);
});
```

**为什么需要延迟？**
- QtNodes 在创建节点时发出 `nodeCreated` 信号
- 场景接收到信号后创建 `NodeGraphicsObject`
- 这个过程是异步的，需要等待下一个事件循环
- 在图形对象创建前设置位置会失败

### QtNodes 坐标系统

QtNodes 使用 Qt 的图形视图框架：
- **视图坐标 (View Coordinates)**: 相对于可见视口的像素坐标
- **场景坐标 (Scene Coordinates)**: 相对于整个场景的逻辑坐标
- 支持缩放、平移等变换

### 节点位置角色 (NodeRole::Position)

QtNodes 通过 `NodeRole` 枚举管理节点属性：
```cpp
enum class NodeRole {
    Type = 0,
    Position = 1,     // QPointF - 节点在场景中的位置
    Size = 2,         // QSize - 节点大小
    CaptionVisible = 3,
    Caption = 4,
    // ...
};
```

## 相关文件

- `src/ui/VisionBoxGraphicsView.h` - 头文件
- `src/ui/VisionBoxGraphicsView.cpp` - 实现文件
- `src/ui/NodePalette.h` - 算子工具箱
- `src/ui/NodePalette.cpp` - 算子工具箱实现

## 测试建议

1. **基本测试**:
   - 拖拽不同算子到画布不同位置
   - 验证节点出现在鼠标释放位置附近

2. **边界测试**:
   - 拖拽到画布边缘
   - 拖拽到画布角落
   - 验证节点不会超出画布范围

3. **性能测试**:
   - 快速连续拖拽多个节点
   - 验证没有性能问题

## 未来改进

1. **动态节点大小**:
   - 根据实际节点大小计算偏移
   - 实现精确的中心对齐

2. **智能吸附**:
   - 节点靠近其他节点时自动对齐
   - 网格吸附功能

3. **碰撞检测**:
   - 避免节点重叠
   - 自动调整位置

4. **替代方案**:
   - 监听 `nodeCreated` 信号后设置位置
   - 优点：更精确的时机控制
   - 缺点：需要额外管理信号连接

## 优化历史

- **2026-01-27**: 初始实现节点位置设置
- **2026-01-27**: 修复第一次拖拽位置不正确问题（添加事件循环延迟）
