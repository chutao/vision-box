# VisionBox 未完成工作清单

生成日期：2026-01-27

## 📋 概述

基于项目需求文档和当前代码状态，以下是尚未完成的关键功能。

---

## 🔴 高优先级（核心功能缺失）

### 1. 流程序列化（保存/加载）
**状态**: ✅ 已完成（2026-01-27）
**文件**: `src/ui/DataFlowGraphModel.cpp`, `src/ui/MainWindow.cpp`

**已完成功能**:
- [x] 保存所有节点信息（类型、位置、参数）
- [x] 保存节点连接关系
- [x] 保存节点数据状态
- [x] 加载并重建完整流程
- [x] 序列化/反序列化错误处理
- [x] 修改跟踪（自动检测图变更）
- [x] 未保存更改提示

**实现细节**:
- `DataFlowGraphModel::save()`: 调用 QtNodes 基类的序列化实现
- `DataFlowGraphModel::load()`: 清除现有节点后加载新图
- `MainWindow::setModified()`: 统一的修改状态管理
- 连接 QtNodes 信号（nodeCreated, connectionCreated 等）跟踪图变更
- 支持 .vbjson 文件格式

**使用方式**:
1. File → New: 创建新图
2. File → Open: 打开现有图文件
3. File → Save: 保存当前图
4. File → Save As: 另存为新文件
5. 关闭时自动提示保存未保存的更改

---

## 🟡 中优先级（用户体验改进）

### 2. 算子工具箱增强
**状态**: ⚠️ 部分完成
**已完成**:
- ✅ 拖拽添加节点
- ✅ 双击添加节点
- ✅ 分类显示
- ✅ 搜索过滤

**待完成**:
- [ ] 节点图标显示（当前使用通用图标）
- [ ] 节点提示信息（tooltip）
- [ ] 节点预览缩略图
- [ ] 最近使用节点列表

---

### 3. 节点位置定位
**状态**: ✅ 已完成（刚刚修复）
**文件**: `src/ui/VisionBoxGraphicsView.cpp`

---

### 4. 图像生成器性能优化
**状态**: ✅ 已完成
**文件**: `plugins/sources/ImageSourcePlugin/ImageGeneratorModel.cpp`

---

## 🟢 低优先级（增强功能）

### 5. 性能优化
**状态**: ❌ 未实现

**待完成**:
- [ ] Profiling 和瓶颈分析
- [ ] 优化 cv::Mat 拷贝
- [ ] 实现智能缓存机制
- [ ] 多线程处理支持
- [ ] GPU 加速（可选）
  - [ ] CUDA 后端接口
  - [ ] OpenCL 后端接口
  - [ ] UMat 支持

---

### 6. UI 增强
**状态**: ❌ 未实现

**待完成**:
- [ ] 插件管理对话框（查看已加载插件）
- [ ] 节点搜索面板（高级搜索）
- [ ] 自定义节点样式
- [ ] 主题切换（深色/浅色模式）
- [ ] 工具栏自定义

---

### 7. 错误处理增强
**状态**: ✅ 已完成（2026-01-27）

**已完成功能**:
- [x] 优雅的错误传播
- [x] 用户友好的错误消息
- [x] 错误恢复机制
- [x] 节点执行状态可视化

**实现细节**:
- 创建了 `src/core/NodeError.h` 错误处理框架
  - `NodeError` 结构：错误分类、消息、技术细节、可恢复性标志
  - `ErrorBuilder` 工具类：快速创建常见错误类型
  - `ErrorHandlingNode` 基类：提供错误管理方法
  - `tryOpenCVOperation()` 模板函数：自动捕获 OpenCV 异常

- 更新了 `BlurModel` 作为错误处理示例
  - 输入验证（缺失、空数据）
  - 参数范围验证
  - OpenCV 异常处理
  - 错误状态可视化

- 支持的错误类别：
  - `InvalidInput`: 输入数据无效或缺失
  - `InvalidParameter`: 参数超出范围
  - `ProcessingError`: 计算过程错误
  - `MemoryError`: 内存分配失败
  - `FileError`: 文件 I/O 错误

- 错误可视化（通过 QtNodes）：
  - Valid（绿色）：节点正常工作
  - Warning（黄色）：可恢复的问题
  - Error（红色）：阻止计算的严重错误

**文档**: `docs/ERROR_HANDLING.md` - 完整的错误处理指南和示例

---

## 📊 已完成的插件统计

### ✅ 全部完成（29个插件，56个节点模型）

#### 源节点（4个）
- ✅ ImageLoaderModel
- ✅ VideoLoaderModel
- ✅ CameraSourceModel
- ✅ ImageGeneratorModel

#### 基础滤波器（4个）
- ✅ BlurModel
- ✅ ThresholdModel
- ✅ MorphologyModel
- ✅ ColorConvertModel

#### 显示节点（1个）
- ✅ ImageViewerModel

#### 边缘检测（4个）
- ✅ CannyModel
- ✅ SobelModel
- ✅ LaplacianModel
- ✅ ScharrModel

#### 几何变换（3个）
- ✅ ResizeModel
- ✅ RotateModel
- ✅ FlipModel

#### 图像算术（5个）
- ✅ AddModel
- ✅ SubtractModel
- ✅ MultiplyModel
- ✅ DivideModel
- ✅ AbsDiffModel
- ✅ BlendModel

#### 颜色调整（2个）
- ✅ BrightnessContrastModel
- ✅ SaturationModel

#### 直方图（1个）
- ✅ HistogramEqualizationModel

#### 图像增强（2个）
- ✅ DenoiseModel
- ✅ SharpenModel

#### 分割（2个）
- ✅ KMeansSegmentationModel
- ✅ ContourFinderModel

#### 特征检测（1个）
- ✅ CornerDetectionModel

#### 模板匹配（1个）
- ✅ TemplateMatchingModel

#### 高级变换（2个）
- ✅ AffineTransformModel
- ✅ PerspectiveTransformModel

#### 特征描述（1个）
- ✅ ORBFeatureModel

#### 特征匹配（3个）
- ✅ SIFTFeaturesModel
- ✅ AKAZEFeaturesModel
- ✅ FeatureMatcherModel

#### 光流（1个）
- ✅ OpticalFlowModel

#### 视频处理（1个）
- ✅ BackgroundSubtractionModel

#### 目标检测（1个）
- ✅ HOGDetectionModel

#### 人脸检测（1个）
- ✅ HaarFaceDetectionModel

#### 机器学习（2个）
- ✅ DNNInferenceModel
- ✅ ObjectTrackerModel

#### 高级检测（2个）
- ✅ YOLOObjectDetectorModel
- ✅ GrabCutSegmentationModel

#### 导出（3个）
- ✅ ImageExporterModel
- ✅ VideoExporterModel
- ✅ DataExporterModel

#### 可视化（3个）
- ✅ BoundingBoxOverlayModel
- ✅ KeypointViewerModel
- ✅ DrawingOverlayModel

#### 高级形态学（1个）
- ✅ AdvancedMorphologyModel

#### 颜色空间（1个）
- ✅ ColorSpaceModel

#### 距离变换（1个）
- ✅ DistanceTransformModel

#### 分水岭分割（1个）
- ✅ WatershedSegmentationModel

#### 形态学（1个）
- ⚠️ ContourPlugin（可能需要检查）

---

## 📝 文档缺失

### 需要编写的文档
- [ ] **ARCHITECTURE.md** - 系统架构文档
- [ ] **PLUGIN_GUIDE.md** - 插件开发指南
- [ ] **API_REFERENCE.md** - API 参考（Doxygen）
- [ ] **USER_GUIDE.md** - 用户手册
- [ ] **TUTORIAL.md** - 入门教程

### 示例和教程
- [ ] 示例流程文件
- [ ] 教程脚本
- [ ] 视频演示

---

## 🧪 测试缺失

### 单元测试
- [ ] 插件管理器测试
- [ ] 节点模型测试
- [ ] 数据类型测试扩展

### 集成测试
- [ ] 完整流程执行测试
- [ ] 序列化往返测试
- [ ] 跨插件兼容性测试

### 性能测试
- [ ] 图像处理吞吐量测试
- [ ] 内存使用分析
- [ ] 渲染帧率测试
- [ ] 启动时间测试

---

## 🚀 部署和打包

### 打包
- [ ] Linux 安装包（deb/rpm）
- [ ] Windows 安装程序
- [ ] macOS .dmg 包
- [ ] Docker 镜像

### CI/CD
- [ ] GitHub Actions 配置
- [ ] 自动化测试
- [ ] 自动部署

---

## 📊 成功标准检查

根据 REQUIREMENTS.md 的成功标准：

| 指标 | 目标 | 当前状态 | 备注 |
|------|------|---------|------|
| 内置节点数 | ≥ 50 | ✅ 56 | 已达成 |
| 测试覆盖率 | > 80% | ❌ 0% | 待实现 |
| 内存泄漏 | Valgrind 清洁 | ❓ 未测试 | 待验证 |
| 启动时间 | < 3 秒 | ❓ 未测量 | 待测量 |
| 渲染帧率 | > 30 FPS | ❓ 未测量 | 待测量 |

---

## 🎯 建议优先级顺序

### 第一优先级（必须完成）
1. **流程序列化** - 保存/加载功能是核心需求
2. **基础文档** - 用户手册和插件开发指南
3. **单元测试** - 确保代码质量

### 第二优先级（重要增强）
4. **错误处理** - 提升稳定性
5. **性能基准测试** - 建立性能基线
6. **UI 优化** - 提升用户体验

### 第三优先级（锦上添花）
7. **高级功能** - 多线程、GPU加速
8. **部署打包** - 便于分发
9. **示例教程** - 帮助用户上手

---

## 📌 需要注意的问题

1. **第一次节点创建返回 ID=0 的问题**
   - 状态：已知问题，已通过信号机制绕过
   - 影响：功能正常，但需要调查根本原因
   - 建议：研究 QtNodes 的 nodeId 生成机制

2. **序列化功能完全缺失**
   - 状态：空实现
   - 影响：无法保存/加载流程
   - 优先级：高

3. **测试覆盖率为零**
   - 状态：仅有基础数据类型测试
   - 影响：代码质量无法保证
   - 优先级：高

---

## 📅 建议的实施计划

### 第1周：序列化功能
- 实现节点图保存
- 实现节点图加载
- 测试序列化功能

### 第2周：文档和测试
- 编写用户手册
- 编写插件开发指南
- 添加单元测试

### 第3周：优化和增强
- 性能基准测试
- 错误处理增强
- UI 细节优化

---

**总结**: 项目基础设施和插件系统已完善，但核心的用户功能（保存/加载）还需要实现。建议优先完成序列化功能，然后补充文档和测试。
