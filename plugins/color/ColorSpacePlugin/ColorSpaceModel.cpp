/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Space Model Implementation
 ******************************************************************************/

#include "ColorSpaceModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ColorSpaceModel::ColorSpaceModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Color space selector
    auto* spaceLayout = new QHBoxLayout();
    spaceLayout->addWidget(new QLabel("Color Space:"));
    m_colorSpaceCombo = new QComboBox();
    m_colorSpaceCombo->addItem("BGR (OpenCV Default)", static_cast<int>(ColorSpace::BGR));
    m_colorSpaceCombo->addItem("RGB", static_cast<int>(ColorSpace::RGB));
    m_colorSpaceCombo->addItem("HSV", static_cast<int>(ColorSpace::HSV));
    m_colorSpaceCombo->addItem("HLS", static_cast<int>(ColorSpace::HLS));
    m_colorSpaceCombo->addItem("Lab (CIE L*a*b*)", static_cast<int>(ColorSpace::Lab));
    m_colorSpaceCombo->addItem("Luv (CIE L*u*v*)", static_cast<int>(ColorSpace::Luv));
    m_colorSpaceCombo->addItem("XYZ (CIE XYZ)", static_cast<int>(ColorSpace::XYZ));
    m_colorSpaceCombo->addItem("YCrCb", static_cast<int>(ColorSpace::YCrCb));
    m_colorSpaceCombo->addItem("Grayscale", static_cast<int>(ColorSpace::Gray));
    spaceLayout->addWidget(m_colorSpaceCombo);
    layout->addLayout(spaceLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_colorSpaceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ColorSpaceModel::onColorSpaceChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ColorSpaceModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ColorSpaceModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ColorSpaceModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ColorSpaceModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    convertColorSpace();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ColorSpaceModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ColorSpaceModel::onColorSpaceChanged(int index)
{
    m_colorSpace = static_cast<ColorSpace>(m_colorSpaceCombo->itemData(index).toInt());
    convertColorSpace();
}

/*******************************************************************************
 * Color Space Conversion
 ******************************************************************************/
void ColorSpaceModel::convertColorSpace()
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
        cv::Mat output;

        switch (m_colorSpace)
        {
            case ColorSpace::BGR:
                // Keep as BGR (OpenCV default)
                output = input.clone();
                break;

            case ColorSpace::RGB:
                // Convert BGR to RGB
                cv::cvtColor(input, output, cv::COLOR_BGR2RGB);
                cv::cvtColor(output, output, cv::COLOR_RGB2BGR);  // Convert back for display
                break;

            case ColorSpace::HSV:
                // Convert BGR to HSV
                cv::cvtColor(input, output, cv::COLOR_BGR2HSV);
                // Convert back to BGR for visualization
                cv::cvtColor(output, output, cv::COLOR_HSV2BGR);
                break;

            case ColorSpace::HLS:
                // Convert BGR to HLS
                cv::cvtColor(input, output, cv::COLOR_BGR2HLS);
                // Convert back to BGR for visualization
                cv::cvtColor(output, output, cv::COLOR_HLS2BGR);
                break;

            case ColorSpace::Lab:
                // Convert BGR to Lab
                cv::cvtColor(input, output, cv::COLOR_BGR2Lab);
                // Convert back to BGR for visualization
                cv::cvtColor(output, output, cv::COLOR_Lab2BGR);
                break;

            case ColorSpace::Luv:
                // Convert BGR to Luv
                cv::cvtColor(input, output, cv::COLOR_BGR2Luv);
                // Convert back to BGR for visualization
                cv::cvtColor(output, output, cv::COLOR_Luv2BGR);
                break;

            case ColorSpace::XYZ:
                // Convert BGR to XYZ
                cv::cvtColor(input, output, cv::COLOR_BGR2XYZ);
                // Convert back to BGR for visualization
                cv::cvtColor(output, output, cv::COLOR_XYZ2BGR);
                break;

            case ColorSpace::YCrCb:
                // Convert BGR to YCrCb
                cv::cvtColor(input, output, cv::COLOR_BGR2YCrCb);
                // Convert back to BGR for visualization
                cv::cvtColor(output, output, cv::COLOR_YCrCb2BGR);
                break;

            case ColorSpace::Gray:
                // Convert to grayscale
                if (input.channels() > 1)
                {
                    cv::cvtColor(input, output, cv::COLOR_BGR2GRAY);
                    // Convert back to BGR for visualization
                    cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);
                }
                else
                {
                    output = input.clone();
                }
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

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ColorSpaceModel::save() const
{
    QJsonObject modelJson;
    modelJson["colorSpace"] = static_cast<int>(m_colorSpace);
    return modelJson;
}

void ColorSpaceModel::load(QJsonObject const& model)
{
    QJsonValue spaceJson = model["colorSpace"];
    if (!spaceJson.isUndefined())
    {
        m_colorSpace = static_cast<ColorSpace>(spaceJson.toInt());
        m_colorSpaceCombo->setCurrentIndex(static_cast<int>(m_colorSpace));
    }

    convertColorSpace();
}

} // namespace VisionBox
