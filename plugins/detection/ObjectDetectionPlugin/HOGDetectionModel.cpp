/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * HOG Detection Model Implementation
 ******************************************************************************/

#include "HOGDetectionModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/opencv_modules.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
HOGDetectionModel::HOGDetectionModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Hit threshold
    auto* hitLayout = new QHBoxLayout();
    hitLayout->addWidget(new QLabel("Hit Threshold:"));
    m_hitThresholdSpin = new QDoubleSpinBox();
    m_hitThresholdSpin->setRange(-10.0, 10.0);
    m_hitThresholdSpin->setSingleStep(0.1);
    m_hitThresholdSpin->setDecimals(2);
    m_hitThresholdSpin->setValue(0.0);
    hitLayout->addWidget(m_hitThresholdSpin);
    layout->addLayout(hitLayout);

    // Win stride
    auto* strideLayout = new QHBoxLayout();
    strideLayout->addWidget(new QLabel("Window Stride:"));
    m_winStrideSpin = new QSpinBox();
    m_winStrideSpin->setRange(4, 32);
    m_winStrideSpin->setSingleStep(4);
    m_winStrideSpin->setValue(8);
    strideLayout->addWidget(m_winStrideSpin);
    layout->addLayout(strideLayout);

    // Padding
    auto* paddingLayout = new QHBoxLayout();
    paddingLayout->addWidget(new QLabel("Padding:"));
    m_paddingSpin = new QSpinBox();
    m_paddingSpin->setRange(0, 32);
    m_paddingSpin->setSingleStep(4);
    m_paddingSpin->setValue(8);
    paddingLayout->addWidget(m_paddingSpin);
    layout->addLayout(paddingLayout);

    // Scale factor
    auto* scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("Scale Factor:"));
    m_scaleSpin = new QDoubleSpinBox();
    m_scaleSpin->setRange(1.01, 2.0);
    m_scaleSpin->setSingleStep(0.05);
    m_scaleSpin->setDecimals(2);
    m_scaleSpin->setValue(1.05);
    scaleLayout->addWidget(m_scaleSpin);
    layout->addLayout(scaleLayout);

    // Mean shift grouping
    m_meanShiftCheck = new QCheckBox("Mean Shift Grouping");
    m_meanShiftCheck->setChecked(false);
    layout->addWidget(m_meanShiftCheck);

    // Draw bounding boxes
    m_drawBoxesCheck = new QCheckBox("Draw Bounding Boxes");
    m_drawBoxesCheck->setChecked(true);
    layout->addWidget(m_drawBoxesCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    // Initialize HOG descriptor with default people detector
    m_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    connect(m_hitThresholdSpin, &QDoubleSpinBox::valueChanged,
            this, &HOGDetectionModel::onHitThresholdChanged);
    connect(m_winStrideSpin, &QSpinBox::valueChanged,
            this, &HOGDetectionModel::onWinStrideChanged);
    connect(m_paddingSpin, &QSpinBox::valueChanged,
            this, &HOGDetectionModel::onPaddingChanged);
    connect(m_scaleSpin, &QDoubleSpinBox::valueChanged,
            this, &HOGDetectionModel::onScaleChanged);
    connect(m_meanShiftCheck, &QCheckBox::stateChanged,
            this, &HOGDetectionModel::onMeanShiftChanged);
    connect(m_drawBoxesCheck, &QCheckBox::stateChanged,
            this, &HOGDetectionModel::onDrawBoxesChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int HOGDetectionModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType HOGDetectionModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> HOGDetectionModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void HOGDetectionModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                  QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    detectObjects();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* HOGDetectionModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void HOGDetectionModel::onHitThresholdChanged(double value)
{
    m_hitThreshold = value;
    detectObjects();
}

void HOGDetectionModel::onWinStrideChanged(int value)
{
    m_winStride = value;
    detectObjects();
}

void HOGDetectionModel::onPaddingChanged(int value)
{
    m_padding = value;
    detectObjects();
}

void HOGDetectionModel::onScaleChanged(double value)
{
    m_scale = value;
    detectObjects();
}

void HOGDetectionModel::onMeanShiftChanged(int state)
{
    m_meanShift = (state == Qt::Checked);
    detectObjects();
}

void HOGDetectionModel::onDrawBoxesChanged(int state)
{
    m_drawBoxes = (state == Qt::Checked);
    detectObjects();
}

/*******************************************************************************
 * Object Detection
 ******************************************************************************/
void HOGDetectionModel::detectObjects()
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
        if (input.channels() > 1)
        {
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = input;
        }

        // Create output image
        cv::Mat output;
        if (input.channels() == 1)
        {
            cv::cvtColor(input, output, cv::COLOR_GRAY2BGR);
        }
        else
        {
            output = input.clone();
        }

        // Perform HOG detection
        std::vector<cv::Rect> foundLocations;
        std::vector<double> weights;

        cv::Size winStride(m_winStride, m_winStride);
        cv::Size padding(m_padding, m_padding);
        cv::Size winSize(64, 128);  // Default HOG window size

        // Detect people in image
        m_hog.detectMultiScale(gray, foundLocations, weights,
                              m_hitThreshold, winStride, padding,
                              m_scale, m_meanShift);

        // Draw bounding boxes
        if (m_drawBoxes)
        {
            for (size_t i = 0; i < foundLocations.size(); ++i)
            {
                const cv::Rect& rect = foundLocations[i];

                // Draw rectangle
                cv::rectangle(output, rect, cv::Scalar(0, 255, 0), 2);  // Green in BGR

                // Add label
                std::string label = "Person " + std::to_string(i + 1);
                int baseline = 0;
                cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                                   0.5, 1, &baseline);

                // Draw label background
                cv::Rect labelRect(rect.x, rect.y - textSize.height - 5,
                                 textSize.width + 10, textSize.height + 10);
                cv::rectangle(output, labelRect, cv::Scalar(0, 255, 0), -1);

                // Draw label text
                cv::putText(output, label,
                          cv::Point(rect.x + 5, rect.y - 5),
                          cv::FONT_HERSHEY_SIMPLEX, 0.5,
                          cv::Scalar(0, 0, 0), 1);
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

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject HOGDetectionModel::save() const
{
    QJsonObject modelJson;
    modelJson["hitThreshold"] = m_hitThreshold;
    modelJson["winStride"] = m_winStride;
    modelJson["padding"] = m_padding;
    modelJson["scale"] = m_scale;
    modelJson["meanShift"] = m_meanShift;
    modelJson["drawBoxes"] = m_drawBoxes;
    return modelJson;
}

void HOGDetectionModel::load(QJsonObject const& model)
{
    QJsonValue hitJson = model["hitThreshold"];
    if (!hitJson.isUndefined())
    {
        m_hitThreshold = hitJson.toDouble();
        m_hitThresholdSpin->setValue(m_hitThreshold);
    }

    QJsonValue strideJson = model["winStride"];
    if (!strideJson.isUndefined())
    {
        m_winStride = strideJson.toInt();
        m_winStrideSpin->setValue(m_winStride);
    }

    QJsonValue paddingJson = model["padding"];
    if (!paddingJson.isUndefined())
    {
        m_padding = paddingJson.toInt();
        m_paddingSpin->setValue(m_padding);
    }

    QJsonValue scaleJson = model["scale"];
    if (!scaleJson.isUndefined())
    {
        m_scale = scaleJson.toDouble();
        m_scaleSpin->setValue(m_scale);
    }

    QJsonValue meanShiftJson = model["meanShift"];
    if (!meanShiftJson.isUndefined())
    {
        m_meanShift = meanShiftJson.toBool();
        m_meanShiftCheck->setChecked(m_meanShift);
    }

    QJsonValue drawJson = model["drawBoxes"];
    if (!drawJson.isUndefined())
    {
        m_drawBoxes = drawJson.toBool();
        m_drawBoxesCheck->setChecked(m_drawBoxes);
    }

    detectObjects();
}

} // namespace VisionBox
