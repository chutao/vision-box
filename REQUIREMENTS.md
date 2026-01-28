# VisionBox - 计算机视觉研究框架
## 需求文档与技术路线

---

## 一、项目概述

### 1.1 项目目标
构建一个基于 OpenCV 和 Qt NodeEditor 的模块化、插件式计算机视觉研究框架，使研究人员能够通过可视化节点界面快速构建、测试和部署计算机视觉处理流程。

### 1.2 核心价值
- **可视化编程**: 通过拖拽节点构建视觉处理流程
- **插件化架构**: 动态加载/卸载功能模块，无需重新编译
- **研究友好**: 快速原型验证，算法对比测试
- **可扩展性**: 第三方开发者可轻松贡献新节点

---

## 二、功能需求

### 2.1 核心功能
| 功能模块 | 描述 |
|---------|------|
| 节点编辑器 | 基于 Qt NodeEditor 的可视化编辑界面 |
| 插件系统 | Qt 插件技术实现动态加载/卸载 |
| 数据流管理 | 自动数据传递和更新机制 |
| 流程序列化 | 保存/加载完整处理流程 |
| 实时预览 | 处理结果的实时可视化 |

### 2.2 节点分类
1. **源节点 (Sources)**
   - 图片加载 (文件、URL)
   - 视频加载 (文件、摄像头)
   - 图像生成器 (测试图、噪声)

2. **预处理节点 (Filters)**
   - 滤波器 (高斯、中值、双边)
   - 阈值化 (二值、自适应、Otsu)
   - 形态学操作 (腐蚀、膨胀、开闭运算)
   - 颜色空间转换

3. **特征节点 (Features)**
   - 边缘检测 (Canny、Sobel、Laplacian)
   - 角点检测 (Harris、Shi-Tomasi、FAST)
   - 特征描述 (ORB、SIFT、SURF)
   - 特征匹配

4. **检测节点 (Detectors)**
   - 人脸检测 (Haar、DNN)
   - 目标检测 (YOLO、SSD)
   - 分割 (Watershed、GrabCut)

5. **变换节点 (Transforms)**
   - 几何变换 (缩放、旋转、翻转)
   - 仿射变换
   - 透视变换

6. **机器学习节点 (ML)**
   - DNN 推理
   - 目标跟踪 (KCF、CSRT)
   - 图像分类

7. **显示节点 (Displays)**
   - 图像查看器
   - 直方图显示
   - 标注可视化 (边界框、关键点)

8. **导出节点 (Exporters)**
   - 图片保存
   - 视频保存
   - 数据导出 (CSV、JSON)

### 2.3 非功能需求
| 类别 | 要求 |
|------|------|
| 性能 | 实时处理 >30 FPS (标准操作) |
| 兼容性 | Linux、Windows、macOS |
| 可扩展性 | 支持第三方插件开发 |
| 易用性 | 2小时内完成基础教程 |
| 稳定性 | 内存泄漏检测通过 |

---

## 三、技术架构

### 3.1 技术栈
```
┌─────────────────────────────────────┐
│  Qt 5.15+ / Qt 6.x (UI 框架)        │
├─────────────────────────────────────┤
│  Qt NodeEditor (节点编辑器)         │
├─────────────────────────────────────┤
│  OpenCV 4.0+ (视觉处理库)           │
├─────────────────────────────────────┤
│  CMake 3.16+ (构建系统)             │
└─────────────────────────────────────┘
```

### 3.2 架构层次
```
应用层 (MainWindow, Scene Management)
    ↓
插件管理层 (PluginManager, Registry)
    ↓
NodeEditor 集成层 (DataFlowGraphModel)
    ↓
核心数据类型层 (ImageData, VideoData...)
    ↓
视觉处理插件层 (各种算法插件)
```

### 3.3 核心类设计

#### 3.3.1 插件接口
```cpp
class IVisionNodePlugin
{
public:
    virtual QString pluginId() const = 0;
    virtual QString pluginName() const = 0;
    virtual QString pluginVersion() const = 0;
    virtual QStringList categories() const = 0;
    virtual std::vector<std::unique_ptr<QtNodes::NodeDelegateModel>>
        createNodeModels() const = 0;
    virtual void initialize() = 0;
    virtual void cleanup() = 0;
};

Q_DECLARE_INTERFACE(IVisionNodePlugin,
    "com.visionbox.IVisionNodePlugin/1.0")
```

#### 3.3.2 核心数据类型
```cpp
// 图像数据类型
class ImageData : public QtNodes::NodeData
{
public:
    ImageData(cv::Mat image);
    NodeDataType type() const override; // 返回 "opencv_image"
    cv::Mat image() const;
    QImage toQImage() const; // 用于显示
private:
    cv::Mat _image;
};

// 检测结果类型
class DetectionData : public QtNodes::NodeData
{
public:
    struct Detection {
        cv::Rect bbox;
        QString label;
        float confidence;
    };
    NodeDataType type() const override; // 返回 "detection"
    std::vector<Detection> detections() const;
private:
    std::vector<Detection> _detections;
};

// 特征点类型
class KeypointData : public QtNodes::NodeData
{
public:
    NodeDataType type() const override; // 返回 "keypoints"
    std::vector<cv::KeyPoint> keypoints() const;
private:
    std::vector<cv::KeyPoint> _keypoints;
};
```

#### 3.3.3 节点模型模板
```cpp
class ImageFilterNode : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    // 节点标识
    QString caption() const override { return "Gaussian Blur"; }
    QString name() const override { return "GaussianBlurNode"; }

    // 端口配置
    unsigned int nPorts(PortType portType) const override;
    NodeDataType dataType(PortType, PortIndex) const override;

    // 数据流
    std::shared_ptr<NodeData> outData(PortIndex) override;
    void setInData(std::shared_ptr<NodeData>, PortIndex) override;

    // UI 参数控制
    QWidget* embeddedWidget() override;

    // 序列化
    QJsonObject save() const override;
    void restore(QJsonObject const&) override;

private:
    std::shared_ptr<ImageData> _inputImage;
    std::shared_ptr<ImageData> _outputImage;
    int _kernelSize = 5;
    QWidget* _widget = nullptr;
};
```

---

## 四、项目结构

```
vision-box/
├── CMakeLists.txt                    # 根构建配置
├── .clang-format                     # 代码格式化配置
├── .clang-tidy                       # 静态分析配置
├── .clang-format-ignore              # 格式化忽略文件
├── README.md
├── docs/                             # 文档
│   ├── ARCHITECTURE.md
│   ├── PLUGIN_GUIDE.md
│   ├── API_REFERENCE.md
│   └── USER_GUIDE.md
│
├── scripts/                          # 工具脚本
│   ├── format_code.sh               # 代码格式化脚本
│   ├── check_format.sh              # 格式检查脚本
│   ├── run_tidy.sh                  # 静态分析脚本
│   └── setup_git_hooks.sh           # Git hooks 设置
│
├── src/                              # 核心应用
│   ├── core/
│   │   ├── PluginInterface.h         # 插件接口定义
│   │   ├── PluginManager.{h,cpp}     # 插件管理器
│   │   ├── VisionDataTypes.{h,cpp}   # 核心数据类型
│   │   └── DataTypes.{h,cpp}         # 基础数据类型
│   │
│   ├── ui/
│   │   ├── MainWindow.{h,cpp}
│   │   ├── NodeEditorView.{h,cpp}
│   │   └── PluginManagerWidget.{h,cpp}
│   │
│   ├── serialization/
│   │   ├── PipelineSerializer.{h,cpp}
│   │   └── PipelineDeserializer.{h,cpp}
│   │
│   └── main.cpp
│
├── plugins/                          # 内置插件
│   ├── sources/
│   │   └── ImageSourcePlugin/
│   │       ├── ImageLoaderModel.{h,cpp}
│   │       ├── VideoLoaderModel.{h,cpp}
│   │       └── CameraSourceModel.{h,cpp}
│   │
│   ├── filters/
│   │   ├── BasicFilterPlugin/
│   │   │   ├── BlurModel.{h,cpp}
│   │   │   ├── ThresholdModel.{h,cpp}
│   │   │   └── MorphologyModel.{h,cpp}
│   │   └── EdgeDetectionPlugin/
│   │
│   ├── features/
│   │   ├── CornerDetectionPlugin/
│   │   └── FeatureDescriptionPlugin/
│   │
│   ├── detectors/
│   │   ├── FaceDetectionPlugin/
│   │   └── ObjectDetectionPlugin/
│   │
│   ├── displays/
│   │   └── ImageViewerPlugin/
│   │
│   └── exporters/
│       └── ImageExporterPlugin/
│
├── external/
│   └── QtNodes/                      # NodeEditor 子模块
│
├── tests/                            # 测试
│   ├── unit/
│   └── integration/
│
├── examples/                         # 示例流程
│   ├── basic_filters/
│   └── face_detection/
│
└── cmake/                            # CMake 模块
    ├── FindOpenCV.cmake
    └── VisionBoxConfig.cmake.in
```

---

## 五、实施路线

### 阶段 1: 基础设施 (第1-2周)
**目标**: 建立核心框架和构建系统

**任务清单**:
- [ ] 创建项目目录结构
- [ ] 配置 CMake 构建系统
- [ ] 集成 Qt NodeEditor 作为子模块
- [ ] 实现核心数据类型 (ImageData, VideoData, DetectionData)
- [ ] 定义插件接口 IVisionNodePlugin
- [ ] 实现 PluginManager 基础框架
- [ ] 创建主窗口和基础 UI
- [ ] 编写数据类型单元测试

**关键文件**:
- `CMakeLists.txt` - 根构建配置
- `src/core/VisionDataTypes.h` - 数据类型定义
- `src/core/PluginInterface.h` - 插件接口
- `src/core/PluginManager.h` - 插件管理器

### 阶段 2: 核心插件 (第3-4周)
**目标**: 实现基本源节点、过滤器和显示节点

**任务清单**:
- [ ] 实现源节点插件
  - [ ] ImageLoaderModel (图片加载)
  - [ ] VideoLoaderModel (视频加载)
  - [ ] CameraSourceModel (摄像头捕获)
- [ ] 实现基础过滤器插件
  - [ ] BlurModel (高斯模糊)
  - [ ] ThresholdModel (阈值化)
  - [ ] MorphologyModel (形态学操作)
  - [ ] ColorConvertModel (颜色转换)
- [ ] 实现显示节点插件
  - [ ] ImageViewerModel (图像查看器)
  - [ ] HistogramModel (直方图)
- [ ] 实现流程序列化 (保存/加载)
- [ ] 创建示例流程

**关键文件**:
- `plugins/sources/ImageSourcePlugin/` - 源节点
- `plugins/filters/BasicFilterPlugin/` - 过滤器
- `plugins/displays/ImageViewerPlugin/` - 显示节点
- `src/serialization/PipelineSerializer.cpp` - 序列化

### 阶段 3: 高级视觉算法 (第5-7周)
**目标**: 实现特征提取和目标检测

**任务清单**:
- [ ] 特征检测插件
  - [ ] CannyEdgeModel (边缘检测)
  - [ ] HarrisCornerModel (角点检测)
  - [ ] ORBModel (ORB 特征)
  - [ ] SIFTModel (SIFT 特征)
  - [ ] FeatureMatcherModel (特征匹配)
- [ ] 目标检测插件
  - [ ] HaarCascadeModel (Haar 级联)
  - [ ] HOGDetectorModel (HOG 检测器)
  - [ ] TemplateMatchingModel (模板匹配)
- [ ] 几何变换插件
  - [ ] ResizeModel (缩放)
  - [ ] RotateModel (旋转)
  - [ ] AffineTransformModel (仿射变换)
- [ ] 高级可视化
  - [ ] BoundingBoxOverlay (边界框标注)
  - [ ] KeypointViewer (关键点可视化)

**关键文件**:
- `plugins/features/` - 特征插件
- `plugins/detectors/` - 检测插件
- `plugins/transforms/` - 变换插件

### 阶段 4: 机器学习集成 (第8-9周)
**目标**: 深度学习和高级 ML 功能

**任务清单**:
- [ ] DNN 框架集成
  - [ ] DNN 模型加载器 (ONNX/TensorFlow/PyTorch)
  - [ ] 预处理节点
  - [ ] 后处理节点
- [ ] 分类插件
  - [ ] ImageClassifierModel (图像分类)
  - [ ] ModelZoo 集成
- [ ] 跟踪插件
  - [ ] KCFTrackerModel
  - [ ] CSRTTrackerModel
  - [ ] MOSSETrackerModel
- [ ] 分割插件
  - [ ] WatershedSegmentationModel
  - [ ] GrabCutModel
  - [ ] DeepLabModel

**关键文件**:
- `plugins/ml/ClassificationPlugin/` - 分类
- `plugins/ml/TrackingPlugin/` - 跟踪
- `plugins/detectors/SegmentationPlugin/` - 分割

### 阶段 5: 优化与完善 (第10-11周)
**目标**: 性能优化和用户体验提升

**任务清单**:
- [ ] 性能优化
  - [ ] Profiling 和瓶颈分析
  - [ ] 优化 cv::Mat 拷贝
  - [ ] 实现缓存机制
  - [ ] 多线程支持
- [ ] GPU 加速 (可选)
  - [ ] CUDA 后端接口
  - [ ] OpenCL 后端接口
  - [ ] GPU 加速节点
- [ ] UI 增强
  - [ ] 插件管理对话框
  - [ ] 节点搜索面板
  - [ ] 自定义节点样式
- [ ] 错误处理
  - [ ] 优雅的错误传播
  - [ ] 用户友好的错误消息

### 阶段 6: 文档与发布 (第12周)
**目标**: 完善文档和准备发布

**任务清单**:
- [ ] 编写文档
  - [ ] 架构文档
  - [ ] 插件开发指南
  - [ ] API 参考 (Doxygen)
  - [ ] 用户手册
- [ ] 创建示例
  - [ ] 示例流程
  - [ ] 教程脚本
  - [ ] 视频演示
- [ ] 打包
  - [ ] 安装包
  - [ ] Docker 镜像
  - [ ] CI/CD 配置
- [ ] 测试
  - [ ] 综合测试套件
  - [ ] 性能基准测试

---

## 六、关键技术决策

### 6.1 代码风格策略
**选择**: Clang 工具链 + Git Hooks 自动化
- **clang-format**: 代码格式化 (缩进、换行、对齐)
- **clang-tidy**: 静态分析 (潜在错误、现代化建议)
- **预提交检查**: Git hooks 自动验证代码风格
- **CI 集成**: 持续集成强制检查

#### 代码风格标准
基于 **LLVM 编码规范**，适配 Qt 风格：

**关键规则**:
- 缩进: 2 空格
- 大括号: Allman 风格 (Qt 标准)
- 指针对齐: `Type* name` (左侧)
- 命名: `camelCase` (方法), `PascalCase` (类), `snake_case` (变量)
- 行宽: 100 字符
- 包含顺序: `.h` → Qt → STL → 项目

**自动化流程**:
```
1. 开发人员编写代码
2. Git pre-commit hook 自动运行 clang-format
3. Git pre-push hook 运行 clang-tidy
4. CI 运行完整检查并生成报告
```

### 6.2 插件架构
**选择**: Qt 插件系统 + 接口设计
- ✅ Qt 原生支持 (QPluginLoader)
- ✅ 运行时动态加载/卸载
- ✅ 类型安全
- ✅ 跨平台

### 6.3 数据流策略
**选择**: 基于推送的数据流
- ✅ 符合 NodeEditor 设计
- ✅ 高效的实时处理
- ✅ 自动传播机制

### 6.4 图像数据处理
**选择**: 智能指针 + 写时复制
- ✅ 高效内存使用
- ✅ 线程安全
- ✅ 最小化拷贝

### 6.5 GPU 加速
**选择**: 可选后端接口
- ✅ CPU 回退兼容
- ✅ 可插拔后端 (CUDA/OpenCL)
- ✅ 用户可选

---

## 七、构建系统配置

### 7.1 依赖项
```cmake
# Qt (5.15+ 或 6.x)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# OpenCV (4.0+)
find_package(OpenCV 4.0 REQUIRED COMPONENTS
    core imgproc imgcodecs videoio highgui
    video features2d objdetect dnn calib3d
)

# NodeEditor (子模块)
add_subdirectory(external/QtNodes)
```

### 7.2 构建选项
```cmake
option(USE_QT6 "Use Qt6 instead of Qt5" ON)
option(ENABLE_CUDA "Enable CUDA support" OFF)
option(ENABLE_OPENCL "Enable OpenCL support" OFF)
option(BUILD_TESTING "Build tests" ON)
```

### 7.3 插件构建模板
```cmake
# 插件元数据
set(PLUGIN_ID "ImageSourcePlugin")
set(PLUGIN_NAME "Image Source Plugin")
set(PLUGIN_VERSION "1.0.0")

# 插件库
add_library(${PLUGIN_ID} SHARED
    ImageSourcePlugin.cpp
    ImageLoaderModel.cpp
    ...
)

# 链接依赖
target_link_libraries(${PLUGIN_ID} PRIVATE
    Qt::Core
    Qt::Widgets
    QtNodes::QtNodes
    ${OpenCV_LIBS}
)
```

---

## 八、验证测试

### 8.1 单元测试
- 数据类型测试 (ImageData 转换、内存管理)
- 插件管理器测试 (加载/卸载、错误处理)
- 节点模型测试 (输入输出、参数边界)

### 8.2 集成测试
- 完整流程执行测试
- 序列化往返测试
- 跨插件兼容性测试

### 8.3 性能测试
- 图像处理吞吐量
- 内存使用分析
- 插件加载时间
- 流程执行时间

### 8.4 成功标准
| 指标 | 目标 |
|------|------|
| 内置节点数 | ≥ 50 |
| 测试覆盖率 | > 80% |
| 内存泄漏 | Valgrind 清洁 |
| 启动时间 | < 3 秒 |
| 渲染帧率 | > 30 FPS |

---

## 九、未来扩展

### 短期 (v1.0 后)
1. Python 插件支持 (PySide)
2. 脚本节点 (Python/JavaScript)
3. 批处理功能
4. 视频导出

### 长期
1. 远程执行 (分布式处理)
2. Web 界面 (WebAssembly)
3. AI 助手 (LLP 生成流程)
4. 云端集成
5. 协作编辑

---

## 十、风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| OpenCV 版本不兼容 | 高 | 严格版本要求，兼容层 |
| 插件 ABI 稳定性 | 高 | 版本化接口，严格 API |
| 性能瓶颈 | 中 | Profiling、迭代优化、GPU |
| 内存泄漏 | 中 | 智能指针、Valgrind 测试 |
| Qt/NodeEditor API 变化 | 中 | 固定版本，监控上游 |

---

## 附录: 关键文件清单

### 核心文件 (优先实现)
1. `src/core/PluginInterface.h` - 插件接口定义
2. `src/core/VisionDataTypes.h` - 核心数据类型
3. `src/core/PluginManager.{h,cpp}` - 插件管理器
4. `CMakeLists.txt` - 根构建配置
5. `plugins/sources/ImageSourcePlugin/ImageLoaderModel.{h,cpp}` - 示例节点

### 文档文件
1. `docs/ARCHITECTURE.md` - 架构文档
2. `docs/PLUGIN_GUIDE.md` - 插件开发指南
3. `docs/USER_GUIDE.md` - 用户手册
4. `README.md` - 项目说明

---

**文档版本**: 1.0
**最后更新**: 2026-01-26
**作者**: VisionBox Project
