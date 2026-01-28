# VisionBox 错误处理指南

## 概述

VisionBox 提供了完整的错误处理机制，帮助开发者构建健壮的节点模型。错误处理系统包括：

- **错误分类**: 按类型分类错误（输入错误、参数错误、处理错误等）
- **错误传播**: 将错误信息传递给 UI 层显示
- **错误可视化**: 在节点上显示错误状态和消息
- **错误恢复**: 标记错误是否可恢复

## 核心组件

### 1. NodeError 结构

`NodeError` 结构包含以下字段：

```cpp
struct NodeError
{
    ErrorCategory category;      // 错误类别
    QString message;             // 用户友好的错误消息
    QString technicalDetails;    // 技术细节（用于调试）
    bool recoverable;            // 是否可恢复
};
```

### 2. ErrorCategory 枚举

```cpp
enum class ErrorCategory
{
    NoError = 0,           // 无错误
    InvalidInput = 1,      // 输入数据无效或缺失
    InvalidParameter = 2,  // 参数超出范围或无效
    ProcessingError = 3,   // 计算过程中的错误
    MemoryError = 4,       // 内存分配失败
    FileError = 5,         // 文件 I/O 错误
    UnknownError = 6       // 未分类错误
};
```

### 3. ErrorBuilder 工具类

提供静态方法快速创建常见错误：

```cpp
// 缺失输入
NodeError error = ErrorBuilder::missingInput("Input Image");

// 无效参数
NodeError error = ErrorBuilder::invalidParameter("Threshold", "Must be positive");

// 参数超出范围
NodeError error = ErrorBuilder::parameterOutOfRange("Size", 50, 1, 31);

// 处理错误
NodeError error = ErrorBuilder::processingError("Image convolution", "Kernel too large");

// 文件错误
NodeError error = ErrorBuilder::fileError("/path/to/file.jpg", "reading");
```

### 4. ErrorHandlingNode 基类

继承此基类以获得错误处理能力：

```cpp
class MyNodeModel : public QtNodes::NodeDelegateModel, public ErrorHandlingNode
{
    // ...
};
```

`ErrorHandlingNode` 提供：
- `setError()`: 设置错误并更新验证状态
- `clearError()`: 清除错误状态
- `tryOpenCVOperation()`: OpenCV 操作的 try-catch 包装器

## 使用示例

### 示例 1：基础错误处理

```cpp
void MyModel::processData()
{
    // 检查输入是否存在
    if (!m_inputData)
    {
        NodeError error = ErrorBuilder::missingInput("Input Data");
        setError(error, this);
        return;
    }

    // 检查输入是否有效
    if (m_inputData->isEmpty())
    {
        NodeError error = ErrorBuilder::invalidInput("Input Data", "Empty data");
        setError(error, this);
        return;
    }

    // 处理成功，清除错误
    clearError();
    Q_EMIT dataUpdated(0);
}
```

### 示例 2：参数验证

```cpp
void MyModel::setParameter(int value)
{
    // 验证参数范围
    if (value < 0 || value > 255)
    {
        NodeError error = ErrorBuilder::parameterOutOfRange(
            "Threshold",
            value,
            0,
            255
        );
        setError(error, this);
        return;
    }

    m_parameter = value;
    clearError();
    processData();
}
```

### 示例 3：OpenCV 异常处理

```cpp
void MyModel::applyFilter()
{
    if (!m_inputImage || m_inputImage->image().empty())
    {
        setError(ErrorBuilder::missingInput("Image"), this);
        return;
    }

    cv::Mat result;

    // 使用 tryOpenCVOperation 包装器
    bool success = tryOpenCVOperation("Gaussian blur",
        [&]()
        {
            cv::GaussianBlur(m_inputImage->image(), result, cv::Size(5, 5), 0);
        },
        this  // 传递 this 以自动更新验证状态
    );

    if (success)
    {
        m_outputImage = std::make_shared<ImageData>(result);
        Q_EMIT dataUpdated(0);
    }
}
```

### 示例 4：自定义错误处理

```cpp
void MyModel::loadImage(const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists())
    {
        NodeError error(
            ErrorCategory::FileError,
            QString("File not found: %1").arg(filePath),
            filePath,
            false  // 不可恢复
        );
        setError(error, this);
        return;
    }

    cv::Mat image = cv::imread(filePath.toStdString());
    if (image.empty())
    {
        setError(
            ErrorBuilder::processingError("Image loading", "Failed to load image"),
            this
        );
        return;
    }

    m_outputImage = std::make_shared<ImageData>(image);
    clearError();
    Q_EMIT dataUpdated(0);
}
```

## 错误可视化

QtNodes 自动显示节点的验证状态：

- **Valid**: 绿色边框，节点正常工作
- **Warning**: 黄色边框，节点有问题但可以继续
- **Error**: 红色边框，显示错误消息

错误消息显示在：
1. 节点标题下方
2. 节点工具提示
3. 状态栏（如果实现）

## 错误状态映射

`NodeError` 自动映射到 QtNodes 的状态：

| ErrorCategory | NodeValidationState | NodeProcessingStatus |
|---------------|---------------------|----------------------|
| NoError | Valid | Updated |
| InvalidInput | Error | Empty |
| InvalidParameter | Warning | Partial (可恢复) / Failed (不可恢复) |
| ProcessingError | Error | Partial (可恢复) / Failed (不可恢复) |
| MemoryError | Error | Failed |
| FileError | Error | Failed |

## 最佳实践

### 1. 及早验证

在处理数据前验证输入：

```cpp
void setInData(std::shared_ptr<QtNodes::NodeData> data, PortIndex port) override
{
    m_input = std::dynamic_pointer_cast<ImageData>(data);

    // 立即验证
    if (!m_input || m_input->image().empty())
    {
        setError(ErrorBuilder::missingInput("Image"), this);
        m_output = nullptr;
        Q_EMIT dataUpdated(0);
        return;
    }

    process();
}
```

### 2. 提供有用的错误消息

```cpp
// 好的错误消息
"Parameter 'Kernel Size' (50) is out of range [1, 31]"

// 不好的错误消息
"Invalid parameter"
```

### 3. 正确设置可恢复性

```cpp
// 参数错误通常可恢复
setError(ErrorBuilder::invalidParameter(...), this);  // recoverable = true

// 内存错误通常不可恢复
setError(ErrorBuilder::memoryError("Allocation failed"), this);  // recoverable = false
```

### 4. 成功时清除错误

```cpp
void processData()
{
    if (!validateInputs())
        return;

    if (process())
    {
        clearError();  // 重要：清除之前的错误
        Q_EMIT dataUpdated(0);
    }
}
```

### 5. 使用 tryOpenCVOperation 包装器

简化 OpenCV 异常处理：

```cpp
tryOpenCVOperation("image processing", [&]() {
    // OpenCV 代码
    cv::process(image);
}, this);
```

## 完整示例

参考 `BlurModel` 的完整实现：

**文件**: `plugins/filters/BasicFilterPlugin/BlurModel.h`
```cpp
class BlurModel : public QtNodes::NodeDelegateModel, public ErrorHandlingNode
{
    // ...
    QtNodes::NodeValidationState validationState() const override;
    // ...
};
```

**文件**: `plugins/filters/BasicFilterPlugin/BlurModel.cpp`
```cpp
void BlurModel::applyBlur()
{
    // 1. 检查输入
    if (!m_inputImage)
    {
        setError(ErrorBuilder::missingInput("Input Image"), this);
        return;
    }

    // 2. 验证参数
    if (m_kernelSize > std::min(input.rows, input.cols))
    {
        setError(ErrorBuilder::parameterOutOfRange(...), this);
        return;
    }

    // 3. 处理（使用异常处理包装器）
    bool success = tryOpenCVOperation("blur operation", [&]() {
        // OpenCV 代码
    }, this);

    // 4. 更新输出
    if (success)
    {
        m_outputImage = std::make_shared<ImageData>(result);
        Q_EMIT dataUpdated(0);
    }
}
```

## 迁移现有代码

### 从旧式错误处理迁移

**旧代码**：
```cpp
try
{
    cv::GaussianBlur(input, output, cv::Size(kernel, kernel), 0);
    m_output = std::make_shared<ImageData>(output);
}
catch (const cv::Exception& e)
{
    m_output = nullptr;
    Q_EMIT dataUpdated(0);
}
```

**新代码**：
```cpp
bool success = tryOpenCVOperation("Gaussian blur", [&]() {
    cv::GaussianBlur(input, output, cv::Size(kernel, kernel), 0);
}, this);

if (success)
{
    m_output = std::make_shared<ImageData>(output);
    Q_EMIT dataUpdated(0);
}
```

## 调试错误

### 启用详细错误输出

在节点中添加调试输出：

```cpp
void MyModel::process()
{
    if (!validateInputs())
    {
        qDebug() << "[MyModel] Validation failed:"
                 << lastError().message()
                 << "|" << lastError().technicalDetails;
        return;
    }

    // ...
}
```

### 查看错误状态

在运行时检查节点状态：
```cpp
qDebug() << "Node validation state:" << validationState().message();
qDebug() << "Node processing status:" << static_cast<int>(processingStatus());
qDebug() << "Last error category:" << static_cast<int>(lastError().category);
```

## 总结

VisionBox 的错误处理系统提供：
- ✅ 类型安全的错误分类
- ✅ 用户友好的错误消息
- ✅ 自动 UI 状态更新
- ✅ 可扩展的架构
- ✅ 简化的异常处理

使用这些工具可以构建更健壮、更易调试的节点模型。
