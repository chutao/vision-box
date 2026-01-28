/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Canny Edge Detection Node Model Implementation
 ******************************************************************************/

#include "CannyModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
CannyModel::CannyModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Threshold 1 (lower threshold)
    auto* t1Layout = new QHBoxLayout();
    t1Layout->addWidget(new QLabel("Threshold 1:"));
    m_threshold1Spin = new QDoubleSpinBox();
    m_threshold1Spin->setRange(0, 500);
    m_threshold1Spin->setValue(50);
    m_threshold1Spin->setSingleStep(10);
    t1Layout->addWidget(m_threshold1Spin);
    layout->addLayout(t1Layout);

    // Threshold 2 (upper threshold)
    auto* t2Layout = new QHBoxLayout();
    t2Layout->addWidget(new QLabel("Threshold 2:"));
    m_threshold2Spin = new QDoubleSpinBox();
    m_threshold2Spin->setRange(0, 500);
    m_threshold2Spin->setValue(150);
    m_threshold2Spin->setSingleStep(10);
    t2Layout->addWidget(m_threshold2Spin);
    layout->addLayout(t2Layout);

    // Aperture size
    auto* apertureLayout = new QHBoxLayout();
    apertureLayout->addWidget(new QLabel("Aperture:"));
    m_apertureSpin = new QSpinBox();
    m_apertureSpin->setRange(3, 7);
    m_apertureSpin->setSingleStep(2);
    m_apertureSpin->setValue(3);
    apertureLayout->addWidget(m_apertureSpin);
    layout->addLayout(apertureLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_threshold1Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CannyModel::onThreshold1Changed);
    connect(m_threshold2Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &CannyModel::onThreshold2Changed);
    connect(m_apertureSpin, &QSpinBox::valueChanged,
            this, &CannyModel::onApertureSizeChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int CannyModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType CannyModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> CannyModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void CannyModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyCanny();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* CannyModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void CannyModel::applyCanny()
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

    // Convert to grayscale if needed
    cv::Mat gray;
    if (input.channels() == 3)
    {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = input;
    }

    // Apply Gaussian blur to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(3, 3), 0);

    // Apply Canny edge detection
    cv::Mat edges;
    cv::Canny(blurred, edges, m_threshold1, m_threshold2, m_apertureSize);

    m_outputImage = std::make_shared<ImageData>(edges);
    Q_EMIT dataUpdated(0);
}

void CannyModel::onThreshold1Changed(double value)
{
    m_threshold1 = value;
    applyCanny();
}

void CannyModel::onThreshold2Changed(double value)
{
    m_threshold2 = value;
    applyCanny();
}

void CannyModel::onApertureSizeChanged(int size)
{
    m_apertureSize = size;
    applyCanny();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject CannyModel::save() const
{
    QJsonObject modelJson;
    modelJson["threshold1"] = m_threshold1;
    modelJson["threshold2"] = m_threshold2;
    modelJson["apertureSize"] = m_apertureSize;
    return modelJson;
}

void CannyModel::load(QJsonObject const& model)
{
    QJsonValue t1Json = model["threshold1"];
    if (!t1Json.isUndefined())
    {
        m_threshold1 = t1Json.toDouble();
        m_threshold1Spin->setValue(m_threshold1);
    }

    QJsonValue t2Json = model["threshold2"];
    if (!t2Json.isUndefined())
    {
        m_threshold2 = t2Json.toDouble();
        m_threshold2Spin->setValue(m_threshold2);
    }

    QJsonValue apertureJson = model["apertureSize"];
    if (!apertureJson.isUndefined())
    {
        m_apertureSize = apertureJson.toInt();
        m_apertureSpin->setValue(m_apertureSize);
    }

    applyCanny();
}

} // namespace VisionBox
