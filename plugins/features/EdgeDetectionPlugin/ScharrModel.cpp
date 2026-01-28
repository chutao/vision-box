/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Scharr Edge Detection Node Model Implementation
 ******************************************************************************/

#include "ScharrModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ScharrModel::ScharrModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Derivative type selector
    auto* derivLayout = new QHBoxLayout();
    derivLayout->addWidget(new QLabel("Derivative:"));
    m_derivativeCombo = new QComboBox();
    m_derivativeCombo->addItem("X Direction", XDerivative);
    m_derivativeCombo->addItem("Y Direction", YDerivative);
    m_derivativeCombo->addItem("Both (Gradient)", BothDerivatives);
    m_derivativeCombo->setCurrentIndex(static_cast<int>(BothDerivatives));
    derivLayout->addWidget(m_derivativeCombo);
    layout->addLayout(derivLayout);

    // Scale selector
    auto* scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("Scale:"));
    m_scaleSpin = new QDoubleSpinBox();
    m_scaleSpin->setRange(0.1, 10.0);
    m_scaleSpin->setSingleStep(0.1);
    m_scaleSpin->setValue(1.0);
    scaleLayout->addWidget(m_scaleSpin);
    layout->addLayout(scaleLayout);

    // Delta selector
    auto* deltaLayout = new QHBoxLayout();
    deltaLayout->addWidget(new QLabel("Delta:"));
    m_deltaSpin = new QDoubleSpinBox();
    m_deltaSpin->setRange(0.0, 100.0);
    m_deltaSpin->setSingleStep(1.0);
    m_deltaSpin->setValue(0.0);
    deltaLayout->addWidget(m_deltaSpin);
    layout->addLayout(deltaLayout);

    // Convert to grayscale checkbox
    m_convertToGrayCheck = new QCheckBox("Convert to Grayscale");
    m_convertToGrayCheck->setChecked(true);
    layout->addWidget(m_convertToGrayCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_derivativeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScharrModel::onDerivativeChanged);
    connect(m_scaleSpin, &QDoubleSpinBox::valueChanged,
            this, &ScharrModel::onScaleChanged);
    connect(m_deltaSpin, &QDoubleSpinBox::valueChanged,
            this, &ScharrModel::onDeltaChanged);
    connect(m_convertToGrayCheck, &QCheckBox::stateChanged,
            this, &ScharrModel::onConvertToGrayChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ScharrModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ScharrModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ScharrModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ScharrModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                             QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyScharr();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ScharrModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ScharrModel::applyScharr()
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
        // Convert to grayscale if needed
        cv::Mat gray;
        if (m_convertToGray && input.channels() > 1)
        {
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = input;
        }

        cv::Mat gradX, gradY;
        cv::Mat absGradX, absGradY;

        // Apply Scharr in X direction (kernel size is always 3 for Scharr)
        cv::Scharr(gray, gradX, CV_16S, 1, 0, m_scale, m_delta,
                   cv::BORDER_DEFAULT);
        cv::convertScaleAbs(gradX, absGradX);

        // Apply Scharr in Y direction
        cv::Scharr(gray, gradY, CV_16S, 0, 1, m_scale, m_delta,
                   cv::BORDER_DEFAULT);
        cv::convertScaleAbs(gradY, absGradY);

        cv::Mat output;
        switch (m_derivativeType)
        {
            case XDerivative:
                output = absGradX;
                break;

            case YDerivative:
                output = absGradY;
                break;

            case BothDerivatives:
                // Combine gradients
                cv::addWeighted(absGradX, 0.5, absGradY, 0.5, 0, output);
                break;
        }

        // Convert back to BGR if input was color
        if (m_convertToGray && input.channels() > 1)
        {
            cv::Mat outputBGR;
            cv::cvtColor(output, outputBGR, cv::COLOR_GRAY2BGR);
            m_outputImage = std::make_shared<ImageData>(outputBGR);
        }
        else
        {
            m_outputImage = std::make_shared<ImageData>(output);
        }

        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void ScharrModel::onDerivativeChanged(int index)
{
    m_derivativeType = static_cast<DerivativeType>(m_derivativeCombo->itemData(index).toInt());
    applyScharr();
}

void ScharrModel::onScaleChanged(double value)
{
    m_scale = value;
    applyScharr();
}

void ScharrModel::onDeltaChanged(double value)
{
    m_delta = value;
    applyScharr();
}

void ScharrModel::onConvertToGrayChanged(int state)
{
    m_convertToGray = (state == Qt::Checked);
    applyScharr();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ScharrModel::save() const
{
    QJsonObject modelJson;
    modelJson["derivativeType"] = static_cast<int>(m_derivativeType);
    modelJson["scale"] = m_scale;
    modelJson["delta"] = m_delta;
    modelJson["convertToGray"] = m_convertToGray;
    return modelJson;
}

void ScharrModel::load(QJsonObject const& model)
{
    QJsonValue derivJson = model["derivativeType"];
    if (!derivJson.isUndefined())
    {
        m_derivativeType = static_cast<DerivativeType>(derivJson.toInt());
        m_derivativeCombo->setCurrentIndex(static_cast<int>(m_derivativeType));
    }

    QJsonValue scaleJson = model["scale"];
    if (!scaleJson.isUndefined())
    {
        m_scale = scaleJson.toDouble();
        m_scaleSpin->setValue(m_scale);
    }

    QJsonValue deltaJson = model["delta"];
    if (!deltaJson.isUndefined())
    {
        m_delta = deltaJson.toDouble();
        m_deltaSpin->setValue(m_delta);
    }

    QJsonValue grayJson = model["convertToGray"];
    if (!grayJson.isUndefined())
    {
        m_convertToGray = grayJson.toBool();
        m_convertToGrayCheck->setChecked(m_convertToGray);
    }

    applyScharr();
}

} // namespace VisionBox
