/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Morphology Filter Node Model Implementation
 ******************************************************************************/

#include "MorphologyModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
MorphologyModel::MorphologyModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Operation selector
    auto* opLayout = new QHBoxLayout();
    opLayout->addWidget(new QLabel("Operation:"));
    m_opCombo = new QComboBox();
    m_opCombo->addItem("Erode", Erode);
    m_opCombo->addItem("Dilate", Dilate);
    m_opCombo->addItem("Open", Open);
    m_opCombo->addItem("Close", Close);
    m_opCombo->addItem("Gradient", Gradient);
    m_opCombo->addItem("Top Hat", Tophat);
    m_opCombo->addItem("Black Hat", Blackhat);
    opLayout->addWidget(m_opCombo);
    layout->addLayout(opLayout);

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

    connect(m_opCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MorphologyModel::onOpChanged);
    connect(m_kernelSizeSpin, &QSpinBox::valueChanged,
            this, &MorphologyModel::onKernelSizeChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int MorphologyModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType MorphologyModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> MorphologyModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void MorphologyModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyMorphology();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* MorphologyModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void MorphologyModel::applyMorphology()
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

    // Ensure kernel size is odd
    int kernelSize = m_kernelSize;
    if (kernelSize % 2 == 0)
    {
        kernelSize += 1;
    }

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
                                               cv::Size(kernelSize, kernelSize));
    cv::Mat output;

    try
    {
        switch (m_operation)
        {
            case Erode:
                cv::erode(input, output, kernel);
                break;
            case Dilate:
                cv::dilate(input, output, kernel);
                break;
            case Open:
                cv::morphologyEx(input, output, cv::MORPH_OPEN, kernel);
                break;
            case Close:
                cv::morphologyEx(input, output, cv::MORPH_CLOSE, kernel);
                break;
            case Gradient:
                cv::morphologyEx(input, output, cv::MORPH_GRADIENT, kernel);
                break;
            case Tophat:
                cv::morphologyEx(input, output, cv::MORPH_TOPHAT, kernel);
                break;
            case Blackhat:
                cv::morphologyEx(input, output, cv::MORPH_BLACKHAT, kernel);
                break;
            case HitMiss:
                cv::morphologyEx(input, output, cv::MORPH_HITMISS, kernel);
                break;
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

void MorphologyModel::onKernelSizeChanged(int size)
{
    m_kernelSize = size;
    applyMorphology();
}

void MorphologyModel::onOpChanged(int index)
{
    m_operation = static_cast<MorphOp>(m_opCombo->itemData(index).toInt());
    applyMorphology();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject MorphologyModel::save() const
{
    QJsonObject modelJson;
    modelJson["kernelSize"] = m_kernelSize;
    modelJson["operation"] = static_cast<int>(m_operation);
    return modelJson;
}

void MorphologyModel::load(QJsonObject const& model)
{
    QJsonValue kernelJson = model["kernelSize"];
    if (!kernelJson.isUndefined())
    {
        m_kernelSize = kernelJson.toInt();
        m_kernelSizeSpin->setValue(m_kernelSize);
    }

    QJsonValue opJson = model["operation"];
    if (!opJson.isUndefined())
    {
        m_operation = static_cast<MorphOp>(opJson.toInt());
        m_opCombo->setCurrentIndex(static_cast<int>(m_operation));
    }

    applyMorphology();
}

} // namespace VisionBox
