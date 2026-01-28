/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Morphology Model Implementation
 ******************************************************************************/

#include "AdvancedMorphologyModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
AdvancedMorphologyModel::AdvancedMorphologyModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Operation selector
    auto* opLayout = new QHBoxLayout();
    opLayout->addWidget(new QLabel("Operation:"));
    m_operationCombo = new QComboBox();
    m_operationCombo->addItem("Top Hat", static_cast<int>(MorphOperation::TopHat));
    m_operationCombo->addItem("Black Hat", static_cast<int>(MorphOperation::BlackHat));
    m_operationCombo->addItem("Gradient", static_cast<int>(MorphOperation::Gradient));
    m_operationCombo->addItem("Hit/Miss", static_cast<int>(MorphOperation::HitMiss));
    opLayout->addWidget(m_operationCombo);
    layout->addLayout(opLayout);

    // Kernel shape selector
    auto* shapeLayout = new QHBoxLayout();
    shapeLayout->addWidget(new QLabel("Kernel Shape:"));
    m_kernelShapeCombo = new QComboBox();
    m_kernelShapeCombo->addItem("Rectangle", 0);
    m_kernelShapeCombo->addItem("Ellipse", 1);
    m_kernelShapeCombo->addItem("Cross", 2);
    shapeLayout->addWidget(m_kernelShapeCombo);
    layout->addLayout(shapeLayout);

    // Kernel size
    auto* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Kernel Size:"));
    m_kernelSizeSpin = new QSpinBox();
    m_kernelSizeSpin->setRange(3, 31);
    m_kernelSizeSpin->setSingleStep(2);
    m_kernelSizeSpin->setValue(5);
    sizeLayout->addWidget(m_kernelSizeSpin);
    layout->addLayout(sizeLayout);

    // Iterations
    auto* iterLayout = new QHBoxLayout();
    iterLayout->addWidget(new QLabel("Iterations:"));
    m_iterationsSpin = new QSpinBox();
    m_iterationsSpin->setRange(1, 10);
    m_iterationsSpin->setValue(1);
    iterLayout->addWidget(m_iterationsSpin);
    layout->addLayout(iterLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdvancedMorphologyModel::onOperationChanged);
    connect(m_kernelShapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdvancedMorphologyModel::onKernelShapeChanged);
    connect(m_kernelSizeSpin, &QSpinBox::valueChanged,
            this, &AdvancedMorphologyModel::onKernelSizeChanged);
    connect(m_iterationsSpin, &QSpinBox::valueChanged,
            this, &AdvancedMorphologyModel::onIterationsChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int AdvancedMorphologyModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1;
    }
    else
    {
        return 1;
    }
}

QtNodes::NodeDataType AdvancedMorphologyModel::dataType(
    QtNodes::PortType portType,
    QtNodes::PortIndex portIndex) const
{
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    return ImageData().type();
}

/*******************************************************************************
 * Data Flow
 ******************************************************************************/
std::shared_ptr<QtNodes::NodeData> AdvancedMorphologyModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void AdvancedMorphologyModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                       QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    applyMorphology();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* AdvancedMorphologyModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void AdvancedMorphologyModel::onOperationChanged(int index)
{
    m_operation = static_cast<MorphOperation>(m_operationCombo->itemData(index).toInt());
    applyMorphology();
}

void AdvancedMorphologyModel::onKernelShapeChanged(int index)
{
    m_kernelShape = m_kernelShapeCombo->itemData(index).toInt();
    applyMorphology();
}

void AdvancedMorphologyModel::onKernelSizeChanged(int value)
{
    m_kernelSize = value;
    applyMorphology();
}

void AdvancedMorphologyModel::onIterationsChanged(int value)
{
    m_iterations = value;
    applyMorphology();
}

/*******************************************************************************
 * Morphology Operations
 ******************************************************************************/
void AdvancedMorphologyModel::applyMorphology()
{
    if (!m_inputImage)
    {
        m_outputImage = nullptr;
        return;
    }

    cv::Mat input = m_inputImage->image();

    if (input.empty())
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
        return;
    }

    try
    {
        // Create kernel based on shape
        cv::Mat kernel;
        int shape = cv::MORPH_RECT;
        if (m_kernelShape == 1)
            shape = cv::MORPH_ELLIPSE;
        else if (m_kernelShape == 2)
            shape = cv::MORPH_CROSS;

        kernel = cv::getStructuringElement(shape, cv::Size(m_kernelSize, m_kernelSize));

        // Convert to grayscale if needed
        cv::Mat gray;
        if (input.channels() > 1)
        {
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = input;
        }

        cv::Mat output;
        int op = cv::MORPH_GRADIENT;

        switch (m_operation)
        {
            case MorphOperation::TopHat:
                // Top hat: difference between input and opening
                // Highlights bright regions smaller than structuring element
                op = cv::MORPH_TOPHAT;
                cv::morphologyEx(gray, output, op, kernel, cv::Point(-1, -1), m_iterations);
                break;

            case MorphOperation::BlackHat:
                // Black hat: difference between closing and input
                // Highlights dark regions smaller than structuring element
                op = cv::MORPH_BLACKHAT;
                cv::morphologyEx(gray, output, op, kernel, cv::Point(-1, -1), m_iterations);
                break;

            case MorphOperation::Gradient:
                // Morphological gradient: difference between dilation and erosion
                // Enhances edges
                op = cv::MORPH_GRADIENT;
                cv::morphologyEx(gray, output, op, kernel, cv::Point(-1, -1), m_iterations);
                break;

            case MorphOperation::HitMiss:
                // Hit or miss: detects specific patterns
                // Useful for detecting corners, endpoints, etc.
                {
                    // Create a simple hit/miss kernel for detecting top-left corners
                    // This is a simplified example - in practice you'd define custom patterns
                    cv::Mat hitMissKernel = (cv::Mat_<int>(3, 3) <<
                        0, 1, 1,
                        0, 1, 1,
                        -1, 0, 0);

                    // For hit/miss, we need binary image
                    cv::Mat binary;
                    cv::threshold(gray, binary, 128, 255, cv::THRESH_BINARY);

                    cv::morphologyEx(binary, output, cv::MORPH_HITMISS, hitMissKernel,
                                   cv::Point(-1, -1), m_iterations);
                }
                break;
        }

        // Convert back to BGR if input was color
        if (input.channels() > 1)
        {
            cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);
        }

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject AdvancedMorphologyModel::save() const
{
    QJsonObject modelJson;
    modelJson["operation"] = static_cast<int>(m_operation);
    modelJson["kernelShape"] = m_kernelShape;
    modelJson["kernelSize"] = m_kernelSize;
    modelJson["iterations"] = m_iterations;
    return modelJson;
}

void AdvancedMorphologyModel::load(QJsonObject const& model)
{
    QJsonValue opJson = model["operation"];
    if (!opJson.isUndefined())
    {
        m_operation = static_cast<MorphOperation>(opJson.toInt());
        m_operationCombo->setCurrentIndex(static_cast<int>(m_operation));
    }

    QJsonValue shapeJson = model["kernelShape"];
    if (!shapeJson.isUndefined())
    {
        m_kernelShape = shapeJson.toInt();
        m_kernelShapeCombo->setCurrentIndex(m_kernelShape);
    }

    QJsonValue sizeJson = model["kernelSize"];
    if (!sizeJson.isUndefined())
    {
        m_kernelSize = sizeJson.toInt();
        m_kernelSizeSpin->setValue(m_kernelSize);
    }

    QJsonValue iterJson = model["iterations"];
    if (!iterJson.isUndefined())
    {
        m_iterations = iterJson.toInt();
        m_iterationsSpin->setValue(m_iterations);
    }

    applyMorphology();
}

} // namespace VisionBox
