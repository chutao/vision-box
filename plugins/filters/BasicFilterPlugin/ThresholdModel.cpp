/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Threshold Filter Node Model Implementation
 ******************************************************************************/

#include "ThresholdModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ThresholdModel::ThresholdModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Threshold type selector
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Type:"));
    m_typeCombo = new QComboBox();
    m_typeCombo->addItem("Binary", Binary);
    m_typeCombo->addItem("Binary Inverted", BinaryInv);
    m_typeCombo->addItem("Otsu", Otsu);
    m_typeCombo->addItem("Adaptive Mean", AdaptiveMean);
    m_typeCombo->addItem("Adaptive Gaussian", AdaptiveGaussian);
    typeLayout->addWidget(m_typeCombo);
    layout->addLayout(typeLayout);

    // Threshold value
    auto* threshLayout = new QHBoxLayout();
    threshLayout->addWidget(new QLabel("Value:"));
    m_thresholdSpin = new QSpinBox();
    m_thresholdSpin->setRange(0, 255);
    m_thresholdSpin->setValue(127);
    threshLayout->addWidget(m_thresholdSpin);
    layout->addLayout(threshLayout);

    // Max value
    auto* maxLayout = new QHBoxLayout();
    maxLayout->addWidget(new QLabel("Max:"));
    m_maxSpin = new QSpinBox();
    m_maxSpin->setRange(0, 255);
    m_maxSpin->setValue(255);
    maxLayout->addWidget(m_maxSpin);
    layout->addLayout(maxLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ThresholdModel::onTypeChanged);
    connect(m_thresholdSpin, &QSpinBox::valueChanged,
            this, &ThresholdModel::onThresholdValueChanged);
    connect(m_maxSpin, &QSpinBox::valueChanged,
            this, &ThresholdModel::onMaxValueChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ThresholdModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ThresholdModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ThresholdModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ThresholdModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyThreshold();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ThresholdModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ThresholdModel::applyThreshold()
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

    cv::Mat output;
    int threshType = static_cast<int>(m_thresholdType);

    try
    {
        if (m_thresholdType == Otsu)
        {
            cv::threshold(gray, output, 0, m_maxValue, cv::THRESH_BINARY | cv::THRESH_OTSU);
        }
        else if (m_thresholdType == AdaptiveMean)
        {
            cv::adaptiveThreshold(gray, output, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                                   cv::THRESH_BINARY, 11, 2);
        }
        else if (m_thresholdType == AdaptiveGaussian)
        {
            cv::adaptiveThreshold(gray, output, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                   cv::THRESH_BINARY, 11, 2);
        }
        else
        {
            cv::threshold(gray, output, m_thresholdValue, m_maxValue, threshType);
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

void ThresholdModel::onThresholdValueChanged(int value)
{
    m_thresholdValue = value;
    applyThreshold();
}

void ThresholdModel::onMaxValueChanged(int value)
{
    m_maxValue = value;
    applyThreshold();
}

void ThresholdModel::onTypeChanged(int index)
{
    m_thresholdType = static_cast<ThresholdType>(m_typeCombo->itemData(index).toInt());

    // Disable threshold spin for adaptive/otsu methods
    bool useThreshold = (m_thresholdType != Otsu &&
                         m_thresholdType != AdaptiveMean &&
                         m_thresholdType != AdaptiveGaussian);
    m_thresholdSpin->setEnabled(useThreshold);

    applyThreshold();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ThresholdModel::save() const
{
    QJsonObject modelJson;
    modelJson["thresholdValue"] = m_thresholdValue;
    modelJson["maxValue"] = m_maxValue;
    modelJson["thresholdType"] = static_cast<int>(m_thresholdType);
    return modelJson;
}

void ThresholdModel::load(QJsonObject const& model)
{
    QJsonValue threshJson = model["thresholdValue"];
    if (!threshJson.isUndefined())
    {
        m_thresholdValue = threshJson.toDouble();
        m_thresholdSpin->setValue(static_cast<int>(m_thresholdValue));
    }

    QJsonValue maxJson = model["maxValue"];
    if (!maxJson.isUndefined())
    {
        m_maxValue = maxJson.toDouble();
        m_maxSpin->setValue(static_cast<int>(m_maxValue));
    }

    QJsonValue typeJson = model["thresholdType"];
    if (!typeJson.isUndefined())
    {
        m_thresholdType = static_cast<ThresholdType>(typeJson.toInt());
        m_typeCombo->setCurrentIndex(static_cast<int>(m_thresholdType));
    }

    applyThreshold();
}

} // namespace VisionBox
