/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Denoise Model Implementation
 ******************************************************************************/

#include "DenoiseModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
DenoiseModel::DenoiseModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Denoise type selector
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Algorithm:"));
    m_typeCombo = new QComboBox();
    m_typeCombo->addItem("Bilateral Filter", Bilateral);
    m_typeCombo->addItem("Non-Local Means", NonLocalMeans);
    m_typeCombo->addItem("Fast NLM (Colored)", FastNlMeans);
    typeLayout->addWidget(m_typeCombo);
    layout->addLayout(typeLayout);

    // Parameter 1
    auto* param1Layout = new QHBoxLayout();
    m_param1Label = new QLabel("Filter Diameter (d):");
    param1Layout->addWidget(m_param1Label);
    m_param1Spin = new QDoubleSpinBox();
    m_param1Spin->setRange(1, 50);
    m_param1Spin->setDecimals(0);
    m_param1Spin->setValue(10);
    m_param1Spin->setSingleStep(1);
    param1Layout->addWidget(m_param1Spin);
    layout->addLayout(param1Layout);

    // Parameter 2
    auto* param2Layout = new QHBoxLayout();
    m_param2Label = new QLabel("Sigma Color:");
    param2Layout->addWidget(m_param2Label);
    m_param2Spin = new QDoubleSpinBox();
    m_param2Spin->setRange(1, 200);
    m_param2Spin->setValue(30);
    m_param2Spin->setSingleStep(1);
    param2Layout->addWidget(m_param2Spin);
    layout->addLayout(param2Layout);

    // Parameter 3
    auto* param3Layout = new QHBoxLayout();
    m_param3Label = new QLabel("Sigma Space/Templ. Size:");
    param3Layout->addWidget(m_param3Label);
    m_param3Spin = new QSpinBox();
    m_param3Spin->setRange(1, 21);
    m_param3Spin->setSingleStep(2);
    m_param3Spin->setValue(7);
    param3Layout->addWidget(m_param3Spin);
    layout->addLayout(param3Layout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DenoiseModel::onTypeChanged);
    connect(m_param1Spin, &QDoubleSpinBox::valueChanged,
            this, &DenoiseModel::onParameter1Changed);
    connect(m_param2Spin, &QDoubleSpinBox::valueChanged,
            this, &DenoiseModel::onParameter2Changed);
    connect(m_param3Spin, &QSpinBox::valueChanged,
            this, &DenoiseModel::onParameter3Changed);

    // Initialize UI based on default type
    onTypeChanged(0);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int DenoiseModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType DenoiseModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> DenoiseModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void DenoiseModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                              QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyDenoise();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* DenoiseModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void DenoiseModel::applyDenoise()
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

        if (m_denoiseType == Bilateral)
        {
            // Bilateral filter
            int d = static_cast<int>(m_param1);
            double sigmaColor = m_param2;
            double sigmaSpace = static_cast<double>(m_param3);

            cv::bilateralFilter(input, output, d, sigmaColor, sigmaSpace);
        }
        else if (m_denoiseType == NonLocalMeans)
        {
            // Non-local means denoising (grayscale only)
            cv::Mat gray;
            if (input.channels() == 3)
            {
                cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
            }
            else
            {
                gray = input;
            }

            float h = static_cast<float>(m_param1);
            int templateWindowSize = m_param3;
            int searchWindowSize = 21;

            cv::Mat denoised;
            cv::fastNlMeansDenoising(gray, denoised, h, templateWindowSize, searchWindowSize);

            if (input.channels() == 3)
            {
                cv::cvtColor(denoised, output, cv::COLOR_GRAY2BGR);
            }
            else
            {
                output = denoised;
            }
        }
        else if (m_denoiseType == FastNlMeans)
        {
            // Fast Non-local means denoising (colored)
            float h = static_cast<float>(m_param1);
            float hColor = static_cast<float>(m_param2);
            int templateWindowSize = m_param3;
            int searchWindowSize = 21;

            cv::fastNlMeansDenoisingColored(input, output, h, hColor,
                                           templateWindowSize, searchWindowSize);
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

void DenoiseModel::onTypeChanged(int index)
{
    m_denoiseType = static_cast<DenoiseType>(m_typeCombo->itemData(index).toInt());

    // Update labels and ranges based on type
    switch (m_denoiseType)
    {
        case Bilateral:
            m_param1Label->setText("Filter Diameter (d):");
            m_param1Spin->setRange(1, 50);
            m_param1Spin->setDecimals(0);
            m_param1Spin->setValue(10);
            m_param2Label->setText("Sigma Color:");
            m_param2Spin->setRange(1, 200);
            m_param2Spin->setValue(30);
            m_param3Label->setText("Sigma Space:");
            m_param3Spin->setRange(1, 50);
            m_param3Spin->setValue(7);
            break;

        case NonLocalMeans:
            m_param1Label->setText("Filter Strength (h):");
            m_param1Spin->setRange(1, 50);
            m_param1Spin->setDecimals(1);
            m_param1Spin->setValue(10);
            m_param2Label->setText("(Not used):");
            m_param2Spin->setEnabled(false);
            m_param3Label->setText("Template Size:");
            m_param3Spin->setRange(3, 21);
            m_param3Spin->setValue(7);
            break;

        case FastNlMeans:
            m_param1Label->setText("Luminance (h):");
            m_param1Spin->setRange(1, 50);
            m_param1Spin->setDecimals(1);
            m_param1Spin->setValue(10);
            m_param2Label->setText("Color (hColor):");
            m_param2Spin->setEnabled(true);
            m_param2Spin->setRange(1, 50);
            m_param2Spin->setValue(10);
            m_param3Label->setText("Template Size:");
            m_param3Spin->setRange(3, 21);
            m_param3Spin->setValue(7);
            break;
    }

    applyDenoise();
}

void DenoiseModel::onParameter1Changed(double value)
{
    m_param1 = value;
    applyDenoise();
}

void DenoiseModel::onParameter2Changed(double value)
{
    m_param2 = value;
    applyDenoise();
}

void DenoiseModel::onParameter3Changed(int value)
{
    m_param3 = value;
    applyDenoise();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject DenoiseModel::save() const
{
    QJsonObject modelJson;
    modelJson["denoiseType"] = static_cast<int>(m_denoiseType);
    modelJson["param1"] = m_param1;
    modelJson["param2"] = m_param2;
    modelJson["param3"] = m_param3;
    return modelJson;
}

void DenoiseModel::load(QJsonObject const& model)
{
    QJsonValue typeJson = model["denoiseType"];
    if (!typeJson.isUndefined())
    {
        m_denoiseType = static_cast<DenoiseType>(typeJson.toInt());
        m_typeCombo->setCurrentIndex(static_cast<int>(m_denoiseType));
    }

    QJsonValue param1Json = model["param1"];
    if (!param1Json.isUndefined())
    {
        m_param1 = param1Json.toDouble();
        m_param1Spin->setValue(m_param1);
    }

    QJsonValue param2Json = model["param2"];
    if (!param2Json.isUndefined())
    {
        m_param2 = param2Json.toDouble();
        m_param2Spin->setValue(m_param2);
    }

    QJsonValue param3Json = model["param3"];
    if (!param3Json.isUndefined())
    {
        m_param3 = param3Json.toInt();
        m_param3Spin->setValue(m_param3);
    }

    applyDenoise();
}

} // namespace VisionBox
