/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Laplacian Edge Detection Node Model Implementation
 ******************************************************************************/

#include "LaplacianModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
LaplacianModel::LaplacianModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Kernel size selector
    auto* kernelLayout = new QHBoxLayout();
    kernelLayout->addWidget(new QLabel("Kernel Size:"));
    m_kernelSizeCombo = new QComboBox();
    m_kernelSizeCombo->addItem("1x1", Size1);
    m_kernelSizeCombo->addItem("3x3", Size3);
    m_kernelSizeCombo->addItem("5x5", Size5);
    m_kernelSizeCombo->addItem("7x7", Size7);
    m_kernelSizeCombo->setCurrentIndex(static_cast<int>(Size3));
    kernelLayout->addWidget(m_kernelSizeCombo);
    layout->addLayout(kernelLayout);

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

    connect(m_kernelSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LaplacianModel::onKernelSizeChanged);
    connect(m_scaleSpin, &QDoubleSpinBox::valueChanged,
            this, &LaplacianModel::onScaleChanged);
    connect(m_deltaSpin, &QDoubleSpinBox::valueChanged,
            this, &LaplacianModel::onDeltaChanged);
    connect(m_convertToGrayCheck, &QCheckBox::stateChanged,
            this, &LaplacianModel::onConvertToGrayChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int LaplacianModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType LaplacianModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> LaplacianModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void LaplacianModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyLaplacian();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* LaplacianModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void LaplacianModel::applyLaplacian()
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

        // Map kernel size enum to actual size
        int ksize;
        switch (m_kernelSize)
        {
            case Size1:
                ksize = 1;
                break;
            case Size3:
                ksize = 3;
                break;
            case Size5:
                ksize = 5;
                break;
            case Size7:
                ksize = 7;
                break;
            default:
                ksize = 3;
                break;
        }

        // Apply Laplacian operator
        cv::Mat laplacian;
        cv::Laplacian(gray, laplacian, CV_16S, ksize, m_scale, m_delta,
                      cv::BORDER_DEFAULT);

        // Convert to absolute values
        cv::Mat absLaplacian;
        cv::convertScaleAbs(laplacian, absLaplacian);

        // Convert back to BGR if input was color
        if (m_convertToGray && input.channels() > 1)
        {
            cv::Mat outputBGR;
            cv::cvtColor(absLaplacian, outputBGR, cv::COLOR_GRAY2BGR);
            m_outputImage = std::make_shared<ImageData>(outputBGR);
        }
        else
        {
            m_outputImage = std::make_shared<ImageData>(absLaplacian);
        }

        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void LaplacianModel::onKernelSizeChanged(int index)
{
    m_kernelSize = static_cast<KernelSize>(m_kernelSizeCombo->itemData(index).toInt());
    applyLaplacian();
}

void LaplacianModel::onScaleChanged(double value)
{
    m_scale = value;
    applyLaplacian();
}

void LaplacianModel::onDeltaChanged(double value)
{
    m_delta = value;
    applyLaplacian();
}

void LaplacianModel::onConvertToGrayChanged(int state)
{
    m_convertToGray = (state == Qt::Checked);
    applyLaplacian();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject LaplacianModel::save() const
{
    QJsonObject modelJson;
    modelJson["kernelSize"] = static_cast<int>(m_kernelSize);
    modelJson["scale"] = m_scale;
    modelJson["delta"] = m_delta;
    modelJson["convertToGray"] = m_convertToGray;
    return modelJson;
}

void LaplacianModel::load(QJsonObject const& model)
{
    QJsonValue kernelJson = model["kernelSize"];
    if (!kernelJson.isUndefined())
    {
        m_kernelSize = static_cast<KernelSize>(kernelJson.toInt());
        m_kernelSizeCombo->setCurrentIndex(static_cast<int>(m_kernelSize));
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

    applyLaplacian();
}

} // namespace VisionBox
