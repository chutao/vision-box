/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Background Subtraction Model Implementation
 ******************************************************************************/

#include "BackgroundSubtractionModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <QPushButton>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
BackgroundSubtractionModel::BackgroundSubtractionModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Algorithm selector
    auto* algoLayout = new QHBoxLayout();
    algoLayout->addWidget(new QLabel("Algorithm:"));
    m_algorithmCombo = new QComboBox();
    m_algorithmCombo->addItem("MOG2", static_cast<int>(Algorithm::MOG2));
    m_algorithmCombo->addItem("KNN", static_cast<int>(Algorithm::KNN));
    algoLayout->addWidget(m_algorithmCombo);
    layout->addLayout(algoLayout);

    // History (for MOG2 and KNN)
    auto* historyLayout = new QHBoxLayout();
    historyLayout->addWidget(new QLabel("History:"));
    m_historySpin = new QSpinBox();
    m_historySpin->setRange(10, 1000);
    m_historySpin->setValue(500);
    historyLayout->addWidget(m_historySpin);
    layout->addLayout(historyLayout);

    // Threshold
    auto* thresholdLayout = new QHBoxLayout();
    thresholdLayout->addWidget(new QLabel("Threshold:"));
    m_thresholdSpin = new QDoubleSpinBox();
    m_thresholdSpin->setRange(0.0, 100.0);
    m_thresholdSpin->setSingleStep(1.0);
    m_thresholdSpin->setDecimals(1);
    m_thresholdSpin->setValue(16.0);
    thresholdLayout->addWidget(m_thresholdSpin);
    layout->addLayout(thresholdLayout);

    // Detect shadows checkbox
    m_detectShadowsCheck = new QCheckBox("Detect Shadows");
    m_detectShadowsCheck->setChecked(true);
    layout->addWidget(m_detectShadowsCheck);

    // Learning rate
    auto* learningLayout = new QHBoxLayout();
    learningLayout->addWidget(new QLabel("Learning Rate:"));
    m_learningRateSpin = new QDoubleSpinBox();
    m_learningRateSpin->setRange(-1.0, 1.0);
    m_learningRateSpin->setSingleStep(0.1);
    m_learningRateSpin->setDecimals(2);
    m_learningRateSpin->setValue(-1.0);
    m_learningRateSpin->setSpecialValueText("Auto");
    learningLayout->addWidget(m_learningRateSpin);
    layout->addLayout(learningLayout);

    // Reset button
    m_resetButton = new QPushButton("Reset Background");
    layout->addWidget(m_resetButton);

    layout->setContentsMargins(5, 5, 5, 5);

    // Initialize background subtractors
    resetBackground();

    connect(m_algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BackgroundSubtractionModel::onAlgorithmChanged);
    connect(m_historySpin, &QSpinBox::valueChanged,
            this, &BackgroundSubtractionModel::onHistoryChanged);
    connect(m_thresholdSpin, &QDoubleSpinBox::valueChanged,
            this, &BackgroundSubtractionModel::onThresholdChanged);
    connect(m_detectShadowsCheck, &QCheckBox::stateChanged,
            this, &BackgroundSubtractionModel::onDetectShadowsChanged);
    connect(m_learningRateSpin, &QDoubleSpinBox::valueChanged,
            this, &BackgroundSubtractionModel::onLearningRateChanged);
    connect(m_resetButton, &QPushButton::clicked,
            this, &BackgroundSubtractionModel::onResetClicked);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int BackgroundSubtractionModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType BackgroundSubtractionModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> BackgroundSubtractionModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void BackgroundSubtractionModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                          QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    applyBackgroundSubtraction();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* BackgroundSubtractionModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void BackgroundSubtractionModel::onAlgorithmChanged(int index)
{
    m_algorithm = static_cast<Algorithm>(m_algorithmCombo->itemData(index).toInt());
    resetBackground();
    applyBackgroundSubtraction();
}

void BackgroundSubtractionModel::onHistoryChanged(int value)
{
    m_history = value;
    resetBackground();
    applyBackgroundSubtraction();
}

void BackgroundSubtractionModel::onThresholdChanged(double value)
{
    m_threshold = value;
    resetBackground();
    applyBackgroundSubtraction();
}

void BackgroundSubtractionModel::onDetectShadowsChanged(int state)
{
    m_detectShadows = (state == Qt::Checked);
    resetBackground();
    applyBackgroundSubtraction();
}

void BackgroundSubtractionModel::onLearningRateChanged(double value)
{
    m_learningRate = value;
    applyBackgroundSubtraction();
}

void BackgroundSubtractionModel::onResetClicked()
{
    resetBackground();
    applyBackgroundSubtraction();
}

/*******************************************************************************
 * Background Management
 ******************************************************************************/
void BackgroundSubtractionModel::resetBackground()
{
    // Reinitialize background subtractors with current parameters
    double varThreshold = m_threshold;
    bool detectShadows = m_detectShadows;

    switch (m_algorithm)
    {
        case Algorithm::MOG2:
            m_mog2 = cv::createBackgroundSubtractorMOG2(m_history, varThreshold, detectShadows);
            m_knn.reset();
            break;

        case Algorithm::KNN:
            m_knn = cv::createBackgroundSubtractorKNN(m_history, varThreshold, detectShadows);
            m_mog2.reset();
            break;
    }
}

/*******************************************************************************
 * Background Subtraction
 ******************************************************************************/
void BackgroundSubtractionModel::applyBackgroundSubtraction()
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

        // Apply background subtraction
        cv::Mat fgMask;

        if (m_algorithm == Algorithm::MOG2 && m_mog2)
        {
            m_mog2->apply(gray, fgMask, m_learningRate);
        }
        else if (m_algorithm == Algorithm::KNN && m_knn)
        {
            m_knn->apply(gray, fgMask, m_learningRate);
        }
        else
        {
            // No subtractor initialized
            m_outputImage = nullptr;
            Q_EMIT dataUpdated(0);
            return;
        }

        // Create visualization
        cv::Mat output;
        if (input.channels() == 1)
        {
            cv::cvtColor(input, output, cv::COLOR_GRAY2BGR);
        }
        else
        {
            output = input.clone();
        }

        // Highlight foreground regions
        // Foreground mask: 0 = background, 255 = foreground, 127 = shadow
        for (int y = 0; y < fgMask.rows; ++y)
        {
            for (int x = 0; x < fgMask.cols; ++x)
            {
                uchar maskValue = fgMask.at<uchar>(y, x);

                if (maskValue == 255)
                {
                    // Foreground - highlight in green
                    cv::Vec3b& pixel = output.at<cv::Vec3b>(y, x);
                    pixel[0] = 0;   // B
                    pixel[1] = 255; // G
                    pixel[2] = 0;   // R
                }
                else if (maskValue == 127 && m_detectShadows)
                {
                    // Shadow - darken slightly
                    cv::Vec3b& pixel = output.at<cv::Vec3b>(y, x);
                    pixel[0] = pixel[0] / 2;
                    pixel[1] = pixel[1] / 2;
                    pixel[2] = pixel[2] / 2;
                }
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
QJsonObject BackgroundSubtractionModel::save() const
{
    QJsonObject modelJson;
    modelJson["algorithm"] = static_cast<int>(m_algorithm);
    modelJson["history"] = m_history;
    modelJson["threshold"] = m_threshold;
    modelJson["detectShadows"] = m_detectShadows;
    modelJson["learningRate"] = m_learningRate;
    return modelJson;
}

void BackgroundSubtractionModel::load(QJsonObject const& model)
{
    QJsonValue algoJson = model["algorithm"];
    if (!algoJson.isUndefined())
    {
        m_algorithm = static_cast<Algorithm>(algoJson.toInt());
        m_algorithmCombo->setCurrentIndex(static_cast<int>(m_algorithm));
    }

    QJsonValue historyJson = model["history"];
    if (!historyJson.isUndefined())
    {
        m_history = historyJson.toInt();
        m_historySpin->setValue(m_history);
    }

    QJsonValue thresholdJson = model["threshold"];
    if (!thresholdJson.isUndefined())
    {
        m_threshold = thresholdJson.toDouble();
        m_thresholdSpin->setValue(m_threshold);
    }

    QJsonValue shadowsJson = model["detectShadows"];
    if (!shadowsJson.isUndefined())
    {
        m_detectShadows = shadowsJson.toBool();
        m_detectShadowsCheck->setChecked(m_detectShadows);
    }

    QJsonValue learningJson = model["learningRate"];
    if (!learningJson.isUndefined())
    {
        m_learningRate = learningJson.toDouble();
        m_learningRateSpin->setValue(m_learningRate);
    }

    resetBackground();
    applyBackgroundSubtraction();
}

} // namespace VisionBox
