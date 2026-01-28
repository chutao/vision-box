/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Corner Detection Model Implementation
 ******************************************************************************/

#include "CornerDetectionModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
CornerDetectionModel::CornerDetectionModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Method selector
    auto* methodLayout = new QHBoxLayout();
    methodLayout->addWidget(new QLabel("Method:"));
    m_methodCombo = new QComboBox();
    m_methodCombo->addItem("Harris", Harris);
    m_methodCombo->addItem("Shi-Tomasi", ShiTomasi);
    methodLayout->addWidget(m_methodCombo);
    layout->addLayout(methodLayout);

    // Quality level
    auto* qualityLayout = new QHBoxLayout();
    qualityLayout->addWidget(new QLabel("Quality:"));
    m_qualitySpin = new QDoubleSpinBox();
    m_qualitySpin->setRange(0.001, 1.0);
    m_qualitySpin->setSingleStep(0.001);
    m_qualitySpin->setDecimals(3);
    m_qualitySpin->setValue(0.01);
    qualityLayout->addWidget(m_qualitySpin);
    layout->addLayout(qualityLayout);

    // Max corners
    auto* maxLayout = new QHBoxLayout();
    maxLayout->addWidget(new QLabel("Max Corners:"));
    m_maxCornersSpin = new QSpinBox();
    m_maxCornersSpin->setRange(1, 1000);
    m_maxCornersSpin->setValue(100);
    maxLayout->addWidget(m_maxCornersSpin);
    layout->addLayout(maxLayout);

    // Min distance
    auto* minDistLayout = new QHBoxLayout();
    minDistLayout->addWidget(new QLabel("Min Distance:"));
    m_minDistanceSpin = new QSpinBox();
    m_minDistanceSpin->setRange(1, 100);
    m_minDistanceSpin->setValue(10);
    minDistLayout->addWidget(m_minDistanceSpin);
    layout->addLayout(minDistLayout);

    // Block size
    auto* blockSizeLayout = new QHBoxLayout();
    blockSizeLayout->addWidget(new QLabel("Block Size:"));
    m_blockSizeSpin = new QSpinBox();
    m_blockSizeSpin->setRange(3, 31);
    m_blockSizeSpin->setSingleStep(2);
    m_blockSizeSpin->setValue(3);
    blockSizeLayout->addWidget(m_blockSizeSpin);
    layout->addLayout(blockSizeLayout);

    // Show corners checkbox
    m_showCornersCheck = new QCheckBox("Show Detected Corners");
    m_showCornersCheck->setChecked(true);
    layout->addWidget(m_showCornersCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CornerDetectionModel::onMethodChanged);
    connect(m_qualitySpin, &QDoubleSpinBox::valueChanged,
            this, &CornerDetectionModel::onQualityLevelChanged);
    connect(m_maxCornersSpin, &QSpinBox::valueChanged,
            this, &CornerDetectionModel::onMaxCornersChanged);
    connect(m_minDistanceSpin, &QSpinBox::valueChanged,
            this, &CornerDetectionModel::onMinDistanceChanged);
    connect(m_blockSizeSpin, &QSpinBox::valueChanged,
            this, &CornerDetectionModel::onBlockSizeChanged);
    connect(m_showCornersCheck, &QCheckBox::stateChanged,
            this, &CornerDetectionModel::onShowCornersChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int CornerDetectionModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType CornerDetectionModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> CornerDetectionModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void CornerDetectionModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                 QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    detectCorners();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* CornerDetectionModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void CornerDetectionModel::detectCorners()
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
        // Convert to grayscale
        cv::Mat gray;
        if (input.channels() > 1)
        {
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = input;
        }

        std::vector<cv::Point2f> corners;

        if (m_method == Harris)
        {
            // Harris corner detection
            cv::Mat harrisResponse;
            cv::cornerHarris(gray, harrisResponse, m_blockSize, 3, 0.04);

            // Normalize and find corners
            cv::Mat harrisNorm;
            cv::normalize(harrisResponse, harrisNorm, 0, 255, cv::NORM_MINMAX);

            // Find local maxima
            std::vector<cv::Point> cornersInt;
            cv::goodFeaturesToTrack(gray, cornersInt, m_maxCorners, m_qualityLevel,
                                    m_minDistance, cv::Mat(), 3, false, 0.04);

            corners.resize(cornersInt.size());
            for (size_t i = 0; i < cornersInt.size(); i++)
            {
                corners[i] = cv::Point2f(static_cast<float>(cornersInt[i].x),
                                          static_cast<float>(cornersInt[i].y));
            }
        }
        else
        {
            // Shi-Tomasi corner detection
            std::vector<cv::Point> cornersInt;
            cv::goodFeaturesToTrack(gray, cornersInt, m_maxCorners, m_qualityLevel,
                                    m_minDistance);

            corners.resize(cornersInt.size());
            for (size_t i = 0; i < cornersInt.size(); i++)
            {
                corners[i] = cv::Point2f(static_cast<float>(cornersInt[i].x),
                                          static_cast<float>(cornersInt[i].y));
            }
        }

        // Draw output
        cv::Mat output;
        if (input.channels() == 1)
        {
            cv::cvtColor(input, output, cv::COLOR_GRAY2BGR);
        }
        else
        {
            output = input.clone();
        }

        if (m_showCorners && !corners.empty())
        {
            // Draw corners as red circles
            for (const auto& corner : corners)
            {
                cv::circle(output, corner, 5, cv::Scalar(0, 0, 255), 2);  // Red in BGR
                cv::circle(output, corner, 1, cv::Scalar(255, 255, 255), -1);  // White center
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

void CornerDetectionModel::onMethodChanged(int index)
{
    m_method = static_cast<CornerMethod>(m_methodCombo->itemData(index).toInt());
    detectCorners();
}

void CornerDetectionModel::onQualityLevelChanged(double value)
{
    m_qualityLevel = value;
    detectCorners();
}

void CornerDetectionModel::onMaxCornersChanged(int value)
{
    m_maxCorners = value;
    detectCorners();
}

void CornerDetectionModel::onMinDistanceChanged(int value)
{
    m_minDistance = value;
    detectCorners();
}

void CornerDetectionModel::onBlockSizeChanged(int value)
{
    m_blockSize = value;
    detectCorners();
}

void CornerDetectionModel::onShowCornersChanged(int state)
{
    m_showCorners = (state == Qt::Checked);
    detectCorners();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject CornerDetectionModel::save() const
{
    QJsonObject modelJson;
    modelJson["method"] = static_cast<int>(m_method);
    modelJson["qualityLevel"] = m_qualityLevel;
    modelJson["maxCorners"] = m_maxCorners;
    modelJson["minDistance"] = m_minDistance;
    modelJson["blockSize"] = m_blockSize;
    modelJson["showCorners"] = m_showCorners;
    return modelJson;
}

void CornerDetectionModel::load(QJsonObject const& model)
{
    QJsonValue methodJson = model["method"];
    if (!methodJson.isUndefined())
    {
        m_method = static_cast<CornerMethod>(methodJson.toInt());
        m_methodCombo->setCurrentIndex(static_cast<int>(m_method));
    }

    QJsonValue qualityJson = model["qualityLevel"];
    if (!qualityJson.isUndefined())
    {
        m_qualityLevel = qualityJson.toDouble();
        m_qualitySpin->setValue(m_qualityLevel);
    }

    QJsonValue maxJson = model["maxCorners"];
    if (!maxJson.isUndefined())
    {
        m_maxCorners = maxJson.toInt();
        m_maxCornersSpin->setValue(m_maxCorners);
    }

    QJsonValue minDistJson = model["minDistance"];
    if (!minDistJson.isUndefined())
    {
        m_minDistance = minDistJson.toInt();
        m_minDistanceSpin->setValue(m_minDistance);
    }

    QJsonValue blockSizeJson = model["blockSize"];
    if (!blockSizeJson.isUndefined())
    {
        m_blockSize = blockSizeJson.toInt();
        m_blockSizeSpin->setValue(m_blockSize);
    }

    QJsonValue showJson = model["showCorners"];
    if (!showJson.isUndefined())
    {
        m_showCorners = showJson.toBool();
        m_showCornersCheck->setChecked(m_showCorners);
    }

    detectCorners();
}

} // namespace VisionBox
