/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Saturation Adjustment Model Implementation
 ******************************************************************************/

#include "SaturationModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
SaturationModel::SaturationModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Saturation slider
    auto* saturationLayout = new QHBoxLayout();
    saturationLayout->addWidget(new QLabel("Saturation:"));
    m_saturationSlider = new QSlider(Qt::Horizontal);
    m_saturationSlider->setRange(-100, 100);
    m_saturationSlider->setValue(0);
    saturationLayout->addWidget(m_saturationSlider);
    layout->addLayout(saturationLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_saturationSlider, &QSlider::valueChanged,
            this, &SaturationModel::onSaturationChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int SaturationModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType SaturationModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> SaturationModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void SaturationModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                 QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applySaturation();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* SaturationModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void SaturationModel::applySaturation()
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
        // Convert to HSV
        cv::Mat hsv;
        cv::cvtColor(input, hsv, cv::COLOR_BGR2HSV);

        // Convert saturation to scale factor
        // -100 to 100 → 0.0 to 2.0
        double alpha = 1.0 + (m_saturation / 100.0);

        // Adjust saturation channel
        std::vector<cv::Mat> hsvChannels;
        cv::split(hsv, hsvChannels);

        hsvChannels[1] *= alpha;
        cv::normalize(hsvChannels[1], hsvChannels[1], 0, 255, cv::NORM_MINMAX);

        cv::merge(hsvChannels, hsv);

        // Convert back to BGR
        cv::Mat output;
        cv::cvtColor(hsv, output, cv::COLOR_HSV2BGR);

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void SaturationModel::onSaturationChanged(int value)
{
    m_saturation = value;
    applySaturation();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject SaturationModel::save() const
{
    QJsonObject modelJson;
    modelJson["saturation"] = m_saturation;
    return modelJson;
}

void SaturationModel::load(QJsonObject const& model)
{
    QJsonValue saturationJson = model["saturation"];
    if (!saturationJson.isUndefined())
    {
        m_saturation = saturationJson.toInt();
        m_saturationSlider->setValue(m_saturation);
    }

    applySaturation();
}

} // namespace VisionBox
