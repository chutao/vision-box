/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Sharpen Model Implementation
 ******************************************************************************/

#include "SharpenModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
SharpenModel::SharpenModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Strength slider
    auto* strengthLayout = new QHBoxLayout();
    strengthLayout->addWidget(new QLabel("Strength:"));
    m_strengthSlider = new QSlider(Qt::Horizontal);
    m_strengthSlider->setRange(0, 50);  // 0.0 to 5.0
    m_strengthSlider->setValue(10);
    strengthLayout->addWidget(m_strengthSlider);
    m_strengthValueLabel = new QLabel("1.0");
    strengthLayout->addWidget(m_strengthValueLabel);
    layout->addLayout(strengthLayout);

    // Radius slider
    auto* radiusLayout = new QHBoxLayout();
    radiusLayout->addWidget(new QLabel("Radius:"));
    m_radiusSlider = new QSlider(Qt::Horizontal);
    m_radiusSlider->setRange(1, 10);
    m_radiusSlider->setValue(1);
    radiusLayout->addWidget(m_radiusSlider);
    m_radiusValueLabel = new QLabel("1");
    radiusLayout->addWidget(m_radiusValueLabel);
    layout->addLayout(radiusLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_strengthSlider, &QSlider::valueChanged,
            this, &SharpenModel::onStrengthChanged);
    connect(m_radiusSlider, &QSlider::valueChanged,
            this, &SharpenModel::onRadiusChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int SharpenModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType SharpenModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> SharpenModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void SharpenModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                              QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applySharpening();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* SharpenModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void SharpenModel::applySharpening()
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
        // Unsharp mask algorithm
        // 1. Create a blurred version of the image
        cv::Mat blurred;
        int gaussianSize = m_radius * 2 + 1;  // Ensure odd size
        cv::GaussianBlur(input, blurred, cv::Size(gaussianSize, gaussianSize), 0);

        // 2. Calculate the difference (mask)
        cv::Mat mask;
        cv::absdiff(input, blurred, mask);

        // 3. Add the weighted mask to the original image
        cv::Mat output;
        cv::addWeighted(input, 1.0, mask, m_strength, 0, output);

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void SharpenModel::onStrengthChanged(int value)
{
    m_strength = value / 10.0;  // Convert to 0.0-5.0 range
    m_strengthValueLabel->setText(QString::number(m_strength, 'f', 1));
    applySharpening();
}

void SharpenModel::onRadiusChanged(int value)
{
    m_radius = value;
    m_radiusValueLabel->setText(QString::number(m_radius));
    applySharpening();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject SharpenModel::save() const
{
    QJsonObject modelJson;
    modelJson["strength"] = m_strength;
    modelJson["radius"] = m_radius;
    return modelJson;
}

void SharpenModel::load(QJsonObject const& model)
{
    QJsonValue strengthJson = model["strength"];
    if (!strengthJson.isUndefined())
    {
        m_strength = strengthJson.toDouble();
        m_strengthSlider->setValue(static_cast<int>(m_strength * 10));
        m_strengthValueLabel->setText(QString::number(m_strength, 'f', 1));
    }

    QJsonValue radiusJson = model["radius"];
    if (!radiusJson.isUndefined())
    {
        m_radius = radiusJson.toInt();
        m_radiusSlider->setValue(m_radius);
        m_radiusValueLabel->setText(QString::number(m_radius));
    }

    applySharpening();
}

} // namespace VisionBox
