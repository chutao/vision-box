/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Histogram Equalization Model Implementation
 ******************************************************************************/

#include "HistogramEqualizationModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
HistogramEqualizationModel::HistogramEqualizationModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Equalization type selector
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Type:"));
    m_typeCombo = new QComboBox();
    m_typeCombo->addItem("Standard", Standard);
    m_typeCombo->addItem("CLAHE (Adaptive)", CLAHE);
    typeLayout->addWidget(m_typeCombo);
    layout->addLayout(typeLayout);

    // Clip limit (for CLAHE)
    auto* clipLayout = new QHBoxLayout();
    clipLayout->addWidget(new QLabel("Clip Limit:"));
    m_clipLimitSpin = new QSpinBox();
    m_clipLimitSpin->setRange(1, 100);
    m_clipLimitSpin->setValue(2);
    m_clipLimitSpin->setSingleStep(1);
    clipLayout->addWidget(m_clipLimitSpin);
    layout->addLayout(clipLayout);

    // Tile size (for CLAHE)
    auto* tileLayout = new QHBoxLayout();
    tileLayout->addWidget(new QLabel("Tile Size:"));
    m_tileSizeSpin = new QSpinBox();
    m_tileSizeSpin->setRange(2, 32);
    m_tileSizeSpin->setValue(8);
    m_tileSizeSpin->setSingleStep(1);
    tileLayout->addWidget(m_tileSizeSpin);
    layout->addLayout(tileLayout);

    // Convert to grayscale checkbox
    m_convertToGrayCheck = new QCheckBox("Convert to Grayscale");
    layout->addWidget(m_convertToGrayCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HistogramEqualizationModel::onTypeChanged);
    connect(m_clipLimitSpin, &QSpinBox::valueChanged,
            this, &HistogramEqualizationModel::onClipLimitChanged);
    connect(m_tileSizeSpin, &QSpinBox::valueChanged,
            this, &HistogramEqualizationModel::onTileSizeChanged);
    connect(m_convertToGrayCheck, &QCheckBox::stateChanged,
            this, &HistogramEqualizationModel::onConvertToGrayChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int HistogramEqualizationModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType HistogramEqualizationModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> HistogramEqualizationModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void HistogramEqualizationModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyEqualization();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* HistogramEqualizationModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void HistogramEqualizationModel::applyEqualization()
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

        cv::Mat output;

        if (m_equalizationType == Standard)
        {
            // Standard histogram equalization
            if (gray.channels() == 1)
            {
                cv::equalizeHist(gray, output);
            }
            else
            {
                // For color images, apply to Y channel of YCrCb
                cv::Mat ycrcb;
                cv::cvtColor(gray, ycrcb, cv::COLOR_BGR2YCrCb);
                std::vector<cv::Mat> channels;
                cv::split(ycrcb, channels);
                cv::equalizeHist(channels[0], channels[0]);
                cv::merge(channels, ycrcb);
                cv::cvtColor(ycrcb, output, cv::COLOR_YCrCb2BGR);
            }
        }
        else
        {
            // CLAHE (Contrast Limited Adaptive Histogram Equalization)
            auto clahe = cv::createCLAHE(m_clipLimit, cv::Size(m_tileSize, m_tileSize));

            if (gray.channels() == 1)
            {
                clahe->apply(gray, output);
            }
            else
            {
                // For color images, apply to Y channel of YCrCb
                cv::Mat ycrcb;
                cv::cvtColor(gray, ycrcb, cv::COLOR_BGR2YCrCb);
                std::vector<cv::Mat> channels;
                cv::split(ycrcb, channels);
                clahe->apply(channels[0], channels[0]);
                cv::merge(channels, ycrcb);
                cv::cvtColor(ycrcb, output, cv::COLOR_YCrCb2BGR);
            }
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

void HistogramEqualizationModel::onTypeChanged(int index)
{
    m_equalizationType = static_cast<EqualizationType>(m_typeCombo->itemData(index).toInt());

    // Enable/disable CLAHE-specific controls
    bool isCLAHE = (m_equalizationType == CLAHE);
    m_clipLimitSpin->setEnabled(isCLAHE);
    m_tileSizeSpin->setEnabled(isCLAHE);

    applyEqualization();
}

void HistogramEqualizationModel::onClipLimitChanged(int value)
{
    m_clipLimit = static_cast<double>(value);
    applyEqualization();
}

void HistogramEqualizationModel::onTileSizeChanged(int value)
{
    m_tileSize = value;
    applyEqualization();
}

void HistogramEqualizationModel::onConvertToGrayChanged(int state)
{
    m_convertToGray = (state == Qt::Checked);
    applyEqualization();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject HistogramEqualizationModel::save() const
{
    QJsonObject modelJson;
    modelJson["equalizationType"] = static_cast<int>(m_equalizationType);
    modelJson["clipLimit"] = m_clipLimit;
    modelJson["tileSize"] = m_tileSize;
    modelJson["convertToGray"] = m_convertToGray;
    return modelJson;
}

void HistogramEqualizationModel::load(QJsonObject const& model)
{
    QJsonValue typeJson = model["equalizationType"];
    if (!typeJson.isUndefined())
    {
        m_equalizationType = static_cast<EqualizationType>(typeJson.toInt());
        m_typeCombo->setCurrentIndex(static_cast<int>(m_equalizationType));
    }

    QJsonValue clipJson = model["clipLimit"];
    if (!clipJson.isUndefined())
    {
        m_clipLimit = clipJson.toDouble();
        m_clipLimitSpin->setValue(static_cast<int>(m_clipLimit));
    }

    QJsonValue tileJson = model["tileSize"];
    if (!tileJson.isUndefined())
    {
        m_tileSize = tileJson.toInt();
        m_tileSizeSpin->setValue(m_tileSize);
    }

    QJsonValue grayJson = model["convertToGray"];
    if (!grayJson.isUndefined())
    {
        m_convertToGray = grayJson.toBool();
        m_convertToGrayCheck->setChecked(m_convertToGray);
    }

    applyEqualization();
}

} // namespace VisionBox
