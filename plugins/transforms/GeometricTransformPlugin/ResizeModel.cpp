/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Resize Transform Node Model Implementation
 ******************************************************************************/

#include "ResizeModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ResizeModel::ResizeModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Resize mode selector
    auto* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Mode:"));
    m_modeCombo = new QComboBox();
    m_modeCombo->addItem("Absolute", Absolute);
    m_modeCombo->addItem("Relative", Relative);
    m_modeCombo->addItem("Fit Width", FitWidth);
    m_modeCombo->addItem("Fit Height", FitHeight);
    modeLayout->addWidget(m_modeCombo);
    layout->addLayout(modeLayout);

    // Width input
    auto* widthLayout = new QHBoxLayout();
    widthLayout->addWidget(new QLabel("Width:"));
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(1, 4096);
    m_widthSpin->setValue(640);
    widthLayout->addWidget(m_widthSpin);
    layout->addLayout(widthLayout);

    // Height input
    auto* heightLayout = new QHBoxLayout();
    heightLayout->addWidget(new QLabel("Height:"));
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(1, 4096);
    m_heightSpin->setValue(480);
    heightLayout->addWidget(m_heightSpin);
    layout->addLayout(heightLayout);

    // Keep aspect ratio checkbox
    m_keepAspectCheck = new QCheckBox("Keep Aspect Ratio");
    layout->addWidget(m_keepAspectCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ResizeModel::onModeChanged);
    connect(m_widthSpin, &QSpinBox::valueChanged,
            this, &ResizeModel::onWidthChanged);
    connect(m_heightSpin, &QSpinBox::valueChanged,
            this, &ResizeModel::onHeightChanged);
    connect(m_keepAspectCheck, &QCheckBox::stateChanged,
            this, &ResizeModel::onKeepAspectChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ResizeModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ResizeModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ResizeModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ResizeModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyResize();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ResizeModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ResizeModel::applyResize()
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

    cv::Size inputSize = input.size();
    cv::Size targetSize;

    try
    {
        switch (m_resizeMode)
        {
            case Absolute:
                targetSize = cv::Size(m_targetWidth, m_targetHeight);
                break;

            case Relative:
                targetSize = cv::Size(
                    static_cast<int>(inputSize.width * m_targetWidth / 100.0),
                    static_cast<int>(inputSize.height * m_targetHeight / 100.0)
                );
                break;

            case FitWidth:
                if (m_keepAspectRatio)
                {
                    targetSize = cv::Size(
                        m_targetWidth,
                        static_cast<int>(inputSize.height *
                            static_cast<double>(m_targetWidth) / inputSize.width)
                    );
                }
                else
                {
                    targetSize = cv::Size(m_targetWidth, inputSize.height);
                }
                break;

            case FitHeight:
                if (m_keepAspectRatio)
                {
                    targetSize = cv::Size(
                        static_cast<int>(inputSize.width *
                            static_cast<double>(m_targetHeight) / inputSize.height),
                        m_targetHeight
                    );
                }
                else
                {
                    targetSize = cv::Size(inputSize.width, m_targetHeight);
                }
                break;
        }

        cv::Mat output;
        cv::resize(input, output, targetSize);

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void ResizeModel::onWidthChanged(int value)
{
    m_targetWidth = value;
    applyResize();
}

void ResizeModel::onHeightChanged(int value)
{
    m_targetHeight = value;
    applyResize();
}

void ResizeModel::onModeChanged(int index)
{
    m_resizeMode = static_cast<ResizeMode>(m_modeCombo->itemData(index).toInt());

    // Update UI based on mode
    switch (m_resizeMode)
    {
        case Relative:
            m_widthSpin->setRange(1, 200);
            m_widthSpin->setValue(50);
            m_heightSpin->setRange(1, 200);
            m_heightSpin->setValue(50);
            break;
        default:
            m_widthSpin->setRange(1, 4096);
            m_widthSpin->setValue(640);
            m_heightSpin->setRange(1, 4096);
            m_heightSpin->setValue(480);
            break;
    }

    applyResize();
}

void ResizeModel::onKeepAspectChanged(int state)
{
    m_keepAspectRatio = (state == Qt::Checked);
    applyResize();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ResizeModel::save() const
{
    QJsonObject modelJson;
    modelJson["targetWidth"] = m_targetWidth;
    modelJson["targetHeight"] = m_targetHeight;
    modelJson["resizeMode"] = static_cast<int>(m_resizeMode);
    modelJson["keepAspectRatio"] = m_keepAspectRatio;
    return modelJson;
}

void ResizeModel::load(QJsonObject const& model)
{
    QJsonValue widthJson = model["targetWidth"];
    if (!widthJson.isUndefined())
    {
        m_targetWidth = widthJson.toInt();
        m_widthSpin->setValue(m_targetWidth);
    }

    QJsonValue heightJson = model["targetHeight"];
    if (!heightJson.isUndefined())
    {
        m_targetHeight = heightJson.toInt();
        m_heightSpin->setValue(m_targetHeight);
    }

    QJsonValue modeJson = model["resizeMode"];
    if (!modeJson.isUndefined())
    {
        m_resizeMode = static_cast<ResizeMode>(modeJson.toInt());
        m_modeCombo->setCurrentIndex(static_cast<int>(m_resizeMode));
    }

    QJsonValue aspectJson = model["keepAspectRatio"];
    if (!aspectJson.isUndefined())
    {
        m_keepAspectRatio = aspectJson.toBool();
        m_keepAspectCheck->setChecked(m_keepAspectRatio);
    }

    applyResize();
}

} // namespace VisionBox
