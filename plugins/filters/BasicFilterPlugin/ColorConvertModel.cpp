/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Conversion Node Model Implementation
 ******************************************************************************/

#include "ColorConvertModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ColorConvertModel::ColorConvertModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Color space selector
    auto* spaceLayout = new QHBoxLayout();
    spaceLayout->addWidget(new QLabel("To:"));
    m_colorSpaceCombo = new QComboBox();
    m_colorSpaceCombo->addItem("Grayscale", Grayscale);
    m_colorSpaceCombo->addItem("RGB", RGB);
    m_colorSpaceCombo->addItem("HSV", HSV);
    m_colorSpaceCombo->addItem("HLS", HLS);
    m_colorSpaceCombo->addItem("YCrCb", YCrCb);
    spaceLayout->addWidget(m_colorSpaceCombo);
    layout->addLayout(spaceLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_colorSpaceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ColorConvertModel::onColorSpaceChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ColorConvertModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ColorConvertModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ColorConvertModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ColorConvertModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    convertColor();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ColorConvertModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ColorConvertModel::convertColor()
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

    cv::Mat output;
    int inputChannels = input.channels();

    try
    {
        // Determine if we need to convert based on input and target
        bool needsConversion = true;

        switch (m_targetColorSpace)
        {
            case Grayscale:
                if (inputChannels == 1)
                {
                    output = input.clone();
                    needsConversion = false;
                }
                else
                {
                    cv::cvtColor(input, output, cv::COLOR_BGR2GRAY);
                }
                break;

            case RGB:
                if (inputChannels == 3)
                {
                    cv::cvtColor(input, output, cv::COLOR_BGR2RGB);
                }
                else
                {
                    output = input.clone();
                    needsConversion = false;
                }
                break;

            case HSV:
                if (inputChannels == 3)
                {
                    cv::cvtColor(input, output, cv::COLOR_BGR2HSV);
                }
                else
                {
                    output = input.clone();
                    needsConversion = false;
                }
                break;

            case HLS:
                if (inputChannels == 3)
                {
                    cv::cvtColor(input, output, cv::COLOR_BGR2HLS);
                }
                else
                {
                    output = input.clone();
                    needsConversion = false;
                }
                break;

            case YCrCb:
                if (inputChannels == 3)
                {
                    cv::cvtColor(input, output, cv::COLOR_BGR2YCrCb);
                }
                else
                {
                    output = input.clone();
                    needsConversion = false;
                }
                break;

            case BGR:
                output = input.clone();
                needsConversion = false;
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

void ColorConvertModel::onColorSpaceChanged(int index)
{
    m_targetColorSpace = static_cast<ColorSpace>(m_colorSpaceCombo->itemData(index).toInt());
    convertColor();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ColorConvertModel::save() const
{
    QJsonObject modelJson;
    modelJson["targetColorSpace"] = static_cast<int>(m_targetColorSpace);
    return modelJson;
}

void ColorConvertModel::load(QJsonObject const& model)
{
    QJsonValue spaceJson = model["targetColorSpace"];
    if (!spaceJson.isUndefined())
    {
        m_targetColorSpace = static_cast<ColorSpace>(spaceJson.toInt());
        m_colorSpaceCombo->setCurrentIndex(static_cast<int>(m_targetColorSpace));
    }

    convertColor();
}

} // namespace VisionBox
