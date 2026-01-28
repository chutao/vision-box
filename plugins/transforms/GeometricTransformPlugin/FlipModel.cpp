/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Flip Transform Node Model Implementation
 ******************************************************************************/

#include "FlipModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
FlipModel::FlipModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Flip mode selector
    auto* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Flip Mode:"));
    m_modeCombo = new QComboBox();
    m_modeCombo->addItem("Horizontal", Horizontal);
    m_modeCombo->addItem("Vertical", Vertical);
    m_modeCombo->addItem("Both", Both);
    m_modeCombo->setCurrentIndex(static_cast<int>(Horizontal));
    modeLayout->addWidget(m_modeCombo);
    layout->addLayout(modeLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FlipModel::onModeChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int FlipModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType FlipModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> FlipModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void FlipModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyFlip();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* FlipModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void FlipModel::applyFlip()
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
        // Map flip mode to OpenCV flag
        int flipCode;
        switch (m_flipMode)
        {
            case Horizontal:
                flipCode = 1;  // Flip around y-axis (horizontal)
                break;
            case Vertical:
                flipCode = 0;  // Flip around x-axis (vertical)
                break;
            case Both:
                flipCode = -1; // Flip around both axes
                break;
            default:
                flipCode = 1;
                break;
        }

        cv::Mat output;
        cv::flip(input, output, flipCode);

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void FlipModel::onModeChanged(int index)
{
    m_flipMode = static_cast<FlipMode>(m_modeCombo->itemData(index).toInt());
    applyFlip();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject FlipModel::save() const
{
    QJsonObject modelJson;
    modelJson["flipMode"] = static_cast<int>(m_flipMode);
    return modelJson;
}

void FlipModel::load(QJsonObject const& model)
{
    QJsonValue modeJson = model["flipMode"];
    if (!modeJson.isUndefined())
    {
        m_flipMode = static_cast<FlipMode>(modeJson.toInt());
        m_modeCombo->setCurrentIndex(static_cast<int>(m_flipMode));
    }

    applyFlip();
}

} // namespace VisionBox
