/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Blur Filter Node Model Implementation
 ******************************************************************************/

#include "BlurModel.h"
#include "core/VisionDataTypes.h"
#include "core/PerformanceMonitor.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
BlurModel::BlurModel()
    : m_outputImage(nullptr)
    , m_validationState()
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Blur type selector
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Type:"));
    m_blurTypeCombo = new QComboBox();
    m_blurTypeCombo->addItem("Gaussian", Gaussian);
    m_blurTypeCombo->addItem("Median", Median);
    typeLayout->addWidget(m_blurTypeCombo);
    layout->addLayout(typeLayout);

    // Kernel size selector
    auto* kernelLayout = new QHBoxLayout();
    kernelLayout->addWidget(new QLabel("Kernel:"));
    m_kernelSizeSpin = new QSpinBox();
    m_kernelSizeSpin->setRange(1, 31);
    m_kernelSizeSpin->setSingleStep(2);
    m_kernelSizeSpin->setValue(5);
    kernelLayout->addWidget(m_kernelSizeSpin);
    layout->addLayout(kernelLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_blurTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BlurModel::onBlurTypeChanged);
    connect(m_kernelSizeSpin, &QSpinBox::valueChanged,
            this, &BlurModel::onKernelSizeChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int BlurModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input port for image
    }
    else
    {
        return 1; // One output port for blurred image
    }
}

QtNodes::NodeDataType BlurModel::dataType(
    QtNodes::PortType portType,
    QtNodes::PortIndex portIndex) const
{
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    return ImageData().type(); // "opencv_image"
}

QString BlurModel::portCaption(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
{
    Q_UNUSED(portIndex);

    if (portType == QtNodes::PortType::In)
    {
        return "Input Image";
    }
    else
    {
        return "Blurred Image";
    }
}

/*******************************************************************************
 * Data Flow
 ******************************************************************************/
std::shared_ptr<QtNodes::NodeData> BlurModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void BlurModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);

    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    // Check for missing input
    if (!m_inputImage || m_inputImage->image().empty())
    {
        m_outputImage = nullptr;
        NodeError error = ErrorBuilder::missingInput("Input Image");
        setError(error, this);
        Q_EMIT dataUpdated(0);
        return;
    }

    applyBlur();
}

/*******************************************************************************
 * Validation
 ******************************************************************************/
QtNodes::NodeValidationState BlurModel::validationState() const
{
    return m_validationState;
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* BlurModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void BlurModel::applyBlur()
{
    if (!m_inputImage)
    {
        m_outputImage = nullptr;
        NodeError error = ErrorBuilder::missingInput("Input Image");
        setError(error, this);
        Q_EMIT dataUpdated(0);
        return;
    }

    cv::Mat input = m_inputImage->image();
    if (input.empty())
    {
        m_outputImage = nullptr;
        NodeError error = ErrorBuilder::invalidInput("Input Image", "Empty image");
        setError(error, this);
        Q_EMIT dataUpdated(0);
        return;
    }

    cv::Mat blurred;

    // Ensure kernel size is odd
    int kernelSize = m_kernelSize;
    if (kernelSize % 2 == 0)
    {
        kernelSize += 1;
    }

    // Validate kernel size
    if (kernelSize > std::min(input.rows, input.cols))
    {
        m_outputImage = nullptr;
        NodeError error = ErrorBuilder::parameterOutOfRange(
            "Kernel Size",
            m_kernelSize,
            1,
            std::min(input.rows, input.cols));
        setError(error, this);
        Q_EMIT dataUpdated(0);
        return;
    }

    // Start performance timer
    PerformanceTimer timer(this, caption());

    // Use error handling wrapper
    bool success = tryOpenCVOperation("blur operation",
        [&]()
        {
            if (m_blurType == Gaussian)
            {
                cv::GaussianBlur(input, blurred, cv::Size(kernelSize, kernelSize), 0);
            }
            else // Median
            {
                cv::medianBlur(input, blurred, kernelSize);
            }
        },
        this);

    if (success)
    {
        m_outputImage = std::make_shared<ImageData>(blurred);
        Q_EMIT dataUpdated(0);
    }
    else
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void BlurModel::onKernelSizeChanged(int size)
{
    m_kernelSize = size;
    applyBlur();
}

void BlurModel::onBlurTypeChanged(int index)
{
    m_blurType = static_cast<BlurType>(m_blurTypeCombo->itemData(index).toInt());
    applyBlur();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject BlurModel::save() const
{
    QJsonObject modelJson;
    modelJson["kernelSize"] = m_kernelSize;
    modelJson["blurType"] = static_cast<int>(m_blurType);
    return modelJson;
}

void BlurModel::load(QJsonObject const& model)
{

    QJsonValue kernelSizeJson = model["kernelSize"];
    if (!kernelSizeJson.isUndefined())
    {
        m_kernelSize = kernelSizeJson.toInt();
        m_kernelSizeSpin->setValue(m_kernelSize);
    }
    else
    {
    }

    QJsonValue blurTypeJson = model["blurType"];
    if (!blurTypeJson.isUndefined())
    {
        m_blurType = static_cast<BlurType>(blurTypeJson.toInt());
        m_blurTypeCombo->setCurrentIndex(static_cast<int>(m_blurType));
    }
    else
    {
    }

    applyBlur();

    // Emit signal to trigger downstream nodes
    Q_EMIT dataUpdated(0);
}

} // namespace VisionBox
