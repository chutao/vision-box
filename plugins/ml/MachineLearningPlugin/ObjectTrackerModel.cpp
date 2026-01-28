/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Object Tracker Model Implementation
 ******************************************************************************/

#include "ObjectTrackerModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/video.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
ObjectTrackerModel::ObjectTrackerModel()
    : m_outputImage(nullptr)
{
    // Create background subtractor
    m_bgSubtractor = cv::createBackgroundSubtractorMOG2();

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Tracker type
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Tracker:"));
    m_trackerTypeCombo = new QComboBox();
    m_trackerTypeCombo->addItem("Motion Detection", MotionDetection);
    m_trackerTypeCombo->addItem("Dense Optical Flow", DenseOpticalFlow);
    m_trackerTypeCombo->addItem("Sparse Optical Flow", SparseOpticalFlow);
    m_trackerTypeCombo->setCurrentIndex(0);
    m_trackerTypeCombo->setMinimumWidth(150);
    typeLayout->addWidget(m_trackerTypeCombo);
    layout->addLayout(typeLayout);

    // Threshold
    auto* thresholdLayout = new QHBoxLayout();
    thresholdLayout->addWidget(new QLabel("Threshold:"));
    m_thresholdSpin = new QDoubleSpinBox();
    m_thresholdSpin->setRange(0.0, 255.0);
    m_thresholdSpin->setSingleStep(5.0);
    m_thresholdSpin->setValue(25.0);
    thresholdLayout->addWidget(m_thresholdSpin);
    layout->addLayout(thresholdLayout);

    // Morphology
    m_morphologyCheck = new QCheckBox("Apply Morphology");
    m_morphologyCheck->setChecked(true);
    layout->addWidget(m_morphologyCheck);

    // Min area
    auto* minAreaLayout = new QHBoxLayout();
    minAreaLayout->addWidget(new QLabel("Min Area:"));
    m_minAreaSpin = new QSpinBox();
    m_minAreaSpin->setRange(1, 10000);
    m_minAreaSpin->setValue(100);
    minAreaLayout->addWidget(m_minAreaSpin);
    layout->addLayout(minAreaLayout);

    // Max area
    auto* maxAreaLayout = new QHBoxLayout();
    maxAreaLayout->addWidget(new QLabel("Max Area:"));
    m_maxAreaSpin = new QSpinBox();
    m_maxAreaSpin->setRange(100, 1000000);
    m_maxAreaSpin->setValue(100000);
    maxAreaLayout->addWidget(m_maxAreaSpin);
    layout->addLayout(maxAreaLayout);

    // Show bbox
    m_showBboxCheck = new QCheckBox("Show Bounding Boxes");
    m_showBboxCheck->setChecked(true);
    layout->addWidget(m_showBboxCheck);

    // Reset button
    m_resetBtn = new QPushButton("Reset Tracker");
    layout->addWidget(m_resetBtn);

    // Status label
    m_statusLabel = new QLabel("Status: Ready");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");
    layout->addWidget(m_statusLabel);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_trackerTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ObjectTrackerModel::onTrackerTypeChanged);
    connect(m_thresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ObjectTrackerModel::onThresholdChanged);
    connect(m_morphologyCheck, &QCheckBox::stateChanged,
            this, &ObjectTrackerModel::onMorphologyChanged);
    connect(m_minAreaSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ObjectTrackerModel::onMinAreaChanged);
    connect(m_maxAreaSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ObjectTrackerModel::onMaxAreaChanged);
    connect(m_showBboxCheck, &QCheckBox::stateChanged,
            this, &ObjectTrackerModel::onShowBboxChanged);
    connect(m_resetBtn, &QPushButton::clicked,
            this, &ObjectTrackerModel::onResetClicked);
}

ObjectTrackerModel::~ObjectTrackerModel()
{
    m_prevFrame.release();
    m_background.release();
    m_bgSubtractor.release();
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ObjectTrackerModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image
    }
    else
    {
        return 1; // One output image
    }
}

QtNodes::NodeDataType ObjectTrackerModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ObjectTrackerModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ObjectTrackerModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                   QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    processFrame();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ObjectTrackerModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ObjectTrackerModel::onTrackerTypeChanged()
{
    m_trackerType = static_cast<TrackerType>(m_trackerTypeCombo->currentData().toInt());
    initializeTracker();
    processFrame();
}

void ObjectTrackerModel::onThresholdChanged(double value)
{
    m_threshold = value;
    processFrame();
}

void ObjectTrackerModel::onMorphologyChanged(int state)
{
    m_morphology = (state == Qt::Checked);
    processFrame();
}

void ObjectTrackerModel::onMinAreaChanged(int value)
{
    m_minArea = value;
    processFrame();
}

void ObjectTrackerModel::onMaxAreaChanged(int value)
{
    m_maxArea = value;
    processFrame();
}

void ObjectTrackerModel::onShowBboxChanged(int state)
{
    m_showBbox = (state == Qt::Checked);
    processFrame();
}

void ObjectTrackerModel::onResetClicked()
{
    initializeTracker();
    processFrame();
}

/*******************************************************************************
 * Object Tracking
 ******************************************************************************/
void ObjectTrackerModel::initializeTracker()
{
    m_prevFrame.release();
    m_background.release();
    m_prevPoints.clear();
    m_initialized = false;
    m_statusLabel->setText("Status: Tracker reset");
}

void ObjectTrackerModel::processFrame()
{
    if (!m_inputImage)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
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
        cv::Mat result;
        switch (m_trackerType)
        {
            case MotionDetection:
                result = detectMotion();
                break;

            case DenseOpticalFlow:
            {
                // Convert to grayscale
                cv::Mat gray;
                cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);

                if (!m_prevFrame.empty() && m_prevFrame.size() == gray.size())
                {
                    // Calculate optical flow
                    cv::Mat flow;
                    cv::calcOpticalFlowFarneback(m_prevFrame, gray, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

                    // Visualize flow
                    result = input.clone();
                    for (int y = 0; y < flow.rows; y += 10)
                    {
                        for (int x = 0; x < flow.cols; x += 10)
                        {
                            const cv::Point2f& fxy = flow.at<cv::Point2f>(y, x);
                            cv::line(result, cv::Point(x, y),
                                   cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)),
                                   cv::Scalar(0, 255, 0), 1);
                        }
                    }

                    m_statusLabel->setText("Status: Optical flow tracking");
                }
                else
                {
                    result = input.clone();
                    m_statusLabel->setText("Status: Initializing optical flow...");
                }

                m_prevFrame = gray;
                m_initialized = true;
                break;
            }

            case SparseOpticalFlow:
            {
                // Convert to grayscale
                cv::Mat gray;
                cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);

                if (m_prevPoints.empty())
                {
                    // Detect good features to track
                    cv::goodFeaturesToTrack(gray, m_prevPoints, 100,
                                           0.01, 10, cv::Mat(), 3, false, 0.04);
                    m_statusLabel->setText(QString("Status: Tracking %1 points")
                                           .arg(m_prevPoints.size()));
                }
                else if (!m_prevFrame.empty() && m_prevFrame.size() == gray.size())
                {
                    // Calculate optical flow
                    std::vector<cv::Point2f> currPoints;
                    std::vector<uchar> status;
                    std::vector<float> err;

                    cv::calcOpticalFlowPyrLK(m_prevFrame, gray, m_prevPoints,
                                              currPoints, status, err);

                    // Visualize tracking
                    result = input.clone();
                    for (size_t i = 0; i < currPoints.size(); ++i)
                    {
                        if (status[i])
                        {
                            cv::circle(result, m_prevPoints[i], 3,
                                     cv::Scalar(0, 255, 0), -1);
                            cv::circle(result, currPoints[i], 3,
                                     cv::Scalar(0, 0, 255), -1);
                            cv::line(result, m_prevPoints[i], currPoints[i],
                                   cv::Scalar(255, 0, 0), 1);
                        }
                    }

                    // Update points
                    m_prevPoints.clear();
                    for (size_t i = 0; i < currPoints.size(); ++i)
                    {
                        if (status[i])
                        {
                            m_prevPoints.push_back(currPoints[i]);
                        }
                    }

                    m_statusLabel->setText(QString("Status: Tracking %1 points")
                                           .arg(m_prevPoints.size()));
                }
                else
                {
                    result = input.clone();
                    m_statusLabel->setText("Status: Reinitializing...");
                    m_prevPoints.clear();
                }

                m_prevFrame = gray;
                m_initialized = true;
                break;
            }
        }

        m_outputImage = std::make_shared<ImageData>(result);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

cv::Mat ObjectTrackerModel::detectMotion()
{
    if (!m_inputImage)
    {
        return cv::Mat();
    }

    cv::Mat input = m_inputImage->image();

    // Convert to grayscale
    cv::Mat gray;
    if (input.channels() > 1)
    {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = input.clone();
    }

    // Apply background subtraction
    cv::Mat fgMask;
    m_bgSubtractor->apply(gray, fgMask);

    // Threshold
    cv::threshold(fgMask, fgMask, m_threshold, 255, cv::THRESH_BINARY);

    // Morphological operations
    if (m_morphology)
    {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);
        cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);
    }

    // Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Draw bounding boxes
    cv::Mat result = input.clone();
    int objectCount = 0;

    for (const auto& contour : contours)
    {
        double area = cv::contourArea(contour);
        if (area >= m_minArea && area <= m_maxArea)
        {
            if (m_showBbox)
            {
                cv::Rect bbox = cv::boundingRect(contour);
                cv::rectangle(result, bbox, cv::Scalar(0, 255, 0), 2);
            }
            objectCount++;
        }
    }

    m_statusLabel->setText(QString("Status: %1 objects detected").arg(objectCount));

    return result;
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ObjectTrackerModel::save() const
{
    QJsonObject modelJson;
    modelJson["trackerType"] = static_cast<int>(m_trackerType);
    modelJson["threshold"] = m_threshold;
    modelJson["morphology"] = m_morphology;
    modelJson["minArea"] = m_minArea;
    modelJson["maxArea"] = m_maxArea;
    modelJson["showBbox"] = m_showBbox;
    return modelJson;
}

void ObjectTrackerModel::load(QJsonObject const& model)
{
    QJsonValue trackerTypeJson = model["trackerType"];
    if (!trackerTypeJson.isUndefined())
    {
        m_trackerType = static_cast<TrackerType>(trackerTypeJson.toInt());
        for (int i = 0; i < m_trackerTypeCombo->count(); ++i)
        {
            if (m_trackerTypeCombo->itemData(i).toInt() == static_cast<int>(m_trackerType))
            {
                m_trackerTypeCombo->blockSignals(true);
                m_trackerTypeCombo->setCurrentIndex(i);
                m_trackerTypeCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue thresholdJson = model["threshold"];
    if (!thresholdJson.isUndefined())
    {
        m_threshold = thresholdJson.toDouble();
        m_thresholdSpin->setValue(m_threshold);
    }

    QJsonValue morphologyJson = model["morphology"];
    if (!morphologyJson.isUndefined())
    {
        m_morphology = morphologyJson.toBool();
        m_morphologyCheck->setChecked(m_morphology);
    }

    QJsonValue minAreaJson = model["minArea"];
    if (!minAreaJson.isUndefined())
    {
        m_minArea = minAreaJson.toInt();
        m_minAreaSpin->setValue(m_minArea);
    }

    QJsonValue maxAreaJson = model["maxArea"];
    if (!maxAreaJson.isUndefined())
    {
        m_maxArea = maxAreaJson.toInt();
        m_maxAreaSpin->setValue(m_maxArea);
    }

    QJsonValue showBboxJson = model["showBbox"];
    if (!showBboxJson.isUndefined())
    {
        m_showBbox = showBboxJson.toBool();
        m_showBboxCheck->setChecked(m_showBbox);
    }

    initializeTracker();
}

} // namespace VisionBox
