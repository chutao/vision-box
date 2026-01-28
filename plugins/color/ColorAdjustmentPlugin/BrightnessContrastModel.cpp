/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Brightness and Contrast Adjustment Model Implementation
 ******************************************************************************/

#include "BrightnessContrastModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
BrightnessContrastModel::BrightnessContrastModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Brightness slider
    auto* brightnessLayout = new QHBoxLayout();
    brightnessLayout->addWidget(new QLabel("Brightness:"));
    m_brightnessSlider = new QSlider(Qt::Horizontal);
    m_brightnessSlider->setRange(-100, 100);
    m_brightnessSlider->setValue(0);
    brightnessLayout->addWidget(m_brightnessSlider);
    layout->addLayout(brightnessLayout);

    // Contrast slider
    auto* contrastLayout = new QHBoxLayout();
    contrastLayout->addWidget(new QLabel("Contrast:"));
    m_contrastSlider = new QSlider(Qt::Horizontal);
    m_contrastSlider->setRange(-100, 100);
    m_contrastSlider->setValue(0);
    contrastLayout->addWidget(m_contrastSlider);
    layout->addLayout(contrastLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_brightnessSlider, &QSlider::valueChanged,
            this, &BrightnessContrastModel::onBrightnessChanged);
    connect(m_contrastSlider, &QSlider::valueChanged,
            this, &BrightnessContrastModel::onContrastChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int BrightnessContrastModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType BrightnessContrastModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> BrightnessContrastModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void BrightnessContrastModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                         QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyAdjustment();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* BrightnessContrastModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void BrightnessContrastModel::applyAdjustment()
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
        // Convert brightness and contrast to alpha and beta
        // Brightness: -100 to 100 → beta: -100 to 100
        // Contrast: -100 to 100 → alpha: 0.0 to 3.0
        double alpha = 1.0 + (m_contrast / 100.0);
        double beta = m_brightness;

        cv::Mat output;
        input.convertTo(output, -1, alpha, beta);

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void BrightnessContrastModel::onBrightnessChanged(int value)
{
    m_brightness = value;
    applyAdjustment();
}

void BrightnessContrastModel::onContrastChanged(int value)
{
    m_contrast = value;
    applyAdjustment();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject BrightnessContrastModel::save() const
{
    QJsonObject modelJson;
    modelJson["brightness"] = m_brightness;
    modelJson["contrast"] = m_contrast;
    return modelJson;
}

void BrightnessContrastModel::load(QJsonObject const& model)
{
    QJsonValue brightnessJson = model["brightness"];
    if (!brightnessJson.isUndefined())
    {
        m_brightness = brightnessJson.toInt();
        m_brightnessSlider->setValue(m_brightness);
    }

    QJsonValue contrastJson = model["contrast"];
    if (!contrastJson.isUndefined())
    {
        m_contrast = contrastJson.toInt();
        m_contrastSlider->setValue(m_contrast);
    }

    applyAdjustment();
}

} // namespace VisionBox
