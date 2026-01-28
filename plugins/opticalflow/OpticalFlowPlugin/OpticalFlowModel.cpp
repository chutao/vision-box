/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Optical Flow Model Implementation
 ******************************************************************************/

#include "OpticalFlowModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/calib3d.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
OpticalFlowModel::OpticalFlowModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Method selector
    auto* methodLayout = new QHBoxLayout();
    methodLayout->addWidget(new QLabel("Method:"));
    m_methodCombo = new QComboBox();
    m_methodCombo->addItem("Lucas-Kanade (Sparse)", static_cast<int>(FlowMethod::LucasKanade));
    m_methodCombo->addItem("Farneback (Dense)", static_cast<int>(FlowMethod::Farneback));
    methodLayout->addWidget(m_methodCombo);
    layout->addLayout(methodLayout);

    // Max corners (for Lucas-Kanade)
    auto* maxCornersLayout = new QHBoxLayout();
    maxCornersLayout->addWidget(new QLabel("Max Corners:"));
    m_maxCornersSpin = new QSpinBox();
    m_maxCornersSpin->setRange(10, 1000);
    m_maxCornersSpin->setValue(100);
    maxCornersLayout->addWidget(m_maxCornersSpin);
    layout->addLayout(maxCornersLayout);

    // Quality level (for Lucas-Kanade)
    auto* qualityLayout = new QHBoxLayout();
    qualityLayout->addWidget(new QLabel("Quality Level:"));
    m_qualityLevelSpin = new QDoubleSpinBox();
    m_qualityLevelSpin->setRange(0.001, 1.0);
    m_qualityLevelSpin->setSingleStep(0.01);
    m_qualityLevelSpin->setDecimals(3);
    m_qualityLevelSpin->setValue(0.01);
    qualityLayout->addWidget(m_qualityLevelSpin);
    layout->addLayout(qualityLayout);

    // Min distance (for Lucas-Kanade)
    auto* minDistLayout = new QHBoxLayout();
    minDistLayout->addWidget(new QLabel("Min Distance:"));
    m_minDistanceSpin = new QDoubleSpinBox();
    m_minDistanceSpin->setRange(1.0, 100.0);
    m_minDistanceSpin->setSingleStep(1.0);
    m_minDistanceSpin->setValue(10.0);
    minDistLayout->addWidget(m_minDistanceSpin);
    layout->addLayout(minDistLayout);

    // Window size (for Farneback)
    auto* windowLayout = new QHBoxLayout();
    windowLayout->addWidget(new QLabel("Window Size:"));
    m_windowSizeSpin = new QSpinBox();
    m_windowSizeSpin->setRange(3, 31);
    m_windowSizeSpin->setSingleStep(2);
    m_windowSizeSpin->setValue(15);
    windowLayout->addWidget(m_windowSizeSpin);
    layout->addLayout(windowLayout);

    // Max level (for both methods)
    auto* maxLevelLayout = new QHBoxLayout();
    maxLevelLayout->addWidget(new QLabel("Max Level:"));
    m_maxLevelSpin = new QSpinBox();
    m_maxLevelSpin->setRange(0, 10);
    m_maxLevelSpin->setValue(3);
    maxLevelLayout->addWidget(m_maxLevelSpin);
    layout->addLayout(maxLevelLayout);

    // Draw flow checkbox
    m_drawFlowCheck = new QCheckBox("Draw Flow");
    m_drawFlowCheck->setChecked(true);
    layout->addWidget(m_drawFlowCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OpticalFlowModel::onMethodChanged);
    connect(m_maxCornersSpin, &QSpinBox::valueChanged,
            this, &OpticalFlowModel::onMaxCornersChanged);
    connect(m_qualityLevelSpin, &QDoubleSpinBox::valueChanged,
            this, &OpticalFlowModel::onQualityLevelChanged);
    connect(m_minDistanceSpin, &QDoubleSpinBox::valueChanged,
            this, &OpticalFlowModel::onMinDistanceChanged);
    connect(m_windowSizeSpin, &QSpinBox::valueChanged,
            this, &OpticalFlowModel::onWindowSizeChanged);
    connect(m_maxLevelSpin, &QSpinBox::valueChanged,
            this, &OpticalFlowModel::onMaxLevelChanged);
    connect(m_drawFlowCheck, &QCheckBox::stateChanged,
            this, &OpticalFlowModel::onDrawFlowChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int OpticalFlowModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType OpticalFlowModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> OpticalFlowModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void OpticalFlowModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                 QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    computeFlow();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* OpticalFlowModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void OpticalFlowModel::onMethodChanged(int index)
{
    m_method = static_cast<FlowMethod>(m_methodCombo->itemData(index).toInt());
    // Reset frame state when method changes
    m_hasPreviousFrame = false;
    m_prevGray.release();
    m_prevPoints.clear();
    computeFlow();
}

void OpticalFlowModel::onMaxCornersChanged(int value)
{
    m_maxCorners = value;
    computeFlow();
}

void OpticalFlowModel::onQualityLevelChanged(double value)
{
    m_qualityLevel = value;
    computeFlow();
}

void OpticalFlowModel::onMinDistanceChanged(double value)
{
    m_minDistance = value;
    computeFlow();
}

void OpticalFlowModel::onWindowSizeChanged(int value)
{
    m_windowSize = value;
    computeFlow();
}

void OpticalFlowModel::onMaxLevelChanged(int value)
{
    m_maxLevel = value;
    computeFlow();
}

void OpticalFlowModel::onDrawFlowChanged(int state)
{
    m_drawFlow = (state == Qt::Checked);
    computeFlow();
}

/*******************************************************************************
 * Optical Flow Computation
 ******************************************************************************/
void OpticalFlowModel::computeFlow()
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
            gray = input.clone();
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

        // Check if we have a previous frame
        if (!m_hasPreviousFrame || m_prevGray.empty() || gray.size() != m_prevGray.size())
        {
            // No previous frame or size mismatch - store current frame and wait for next
            gray.copyTo(m_prevGray);
            m_hasPreviousFrame = true;

            // For Lucas-Kanade, detect good features to track
            if (m_method == FlowMethod::LucasKanade)
            {
                std::vector<cv::Point2f> corners;
                cv::goodFeaturesToTrack(gray, corners, m_maxCorners, m_qualityLevel,
                                       m_minDistance);

                // Refine corners to sub-pixel accuracy
                if (!corners.empty())
                {
                    cv::Size winSize(10, 10);
                    cv::Size zeroZone(-1, -1);
                    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 20, 0.03);

                    cv::cornerSubPix(gray, corners, winSize, zeroZone, criteria);
                }

                m_prevPoints = corners;
            }

            m_outputImage = std::make_shared<ImageData>(output);
            Q_EMIT dataUpdated(0);
            return;
        }

        // Compute optical flow
        if (m_method == FlowMethod::LucasKanade)
        {
            // Lucas-Kanade Sparse Optical Flow
            if (m_prevPoints.empty())
            {
                // No previous points - detect new ones
                cv::goodFeaturesToTrack(gray, m_prevPoints, m_maxCorners, m_qualityLevel,
                                       m_minDistance);
            }

            if (!m_prevPoints.empty())
            {
                std::vector<cv::Point2f> nextPoints;
                std::vector<uchar> status;
                std::vector<float> err;

                cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.01);
                cv::Size winSize(21, 21);

                cv::calcOpticalFlowPyrLK(m_prevGray, gray, m_prevPoints, nextPoints,
                                        status, err, winSize, m_maxLevel, criteria);

                // Draw flow vectors
                if (m_drawFlow)
                {
                    for (size_t i = 0; i < m_prevPoints.size(); ++i)
                    {
                        if (status[i])
                        {
                            // Draw line from previous to current point
                            cv::line(output, m_prevPoints[i], nextPoints[i],
                                   cv::Scalar(0, 255, 0), 2);  // Green in BGR

                            // Draw circle at current point
                            cv::circle(output, nextPoints[i], 3,
                                     cv::Scalar(0, 0, 255), -1);  // Red filled circle
                        }
                    }
                }

                // Update points for next frame
                std::vector<cv::Point2f> newPoints;
                for (size_t i = 0; i < m_prevPoints.size(); ++i)
                {
                    if (status[i])
                    {
                        newPoints.push_back(nextPoints[i]);
                    }
                }

                // Add new points if needed
                if (newPoints.size() < static_cast<size_t>(m_maxCorners))
                {
                    std::vector<cv::Point2f> newCorners;
                    int needed = m_maxCorners - static_cast<int>(newPoints.size());

                    // Create mask to avoid detecting near existing points
                    cv::Mat mask = cv::Mat::ones(gray.size(), CV_8UC1) * 255;
                    for (const auto& pt : newPoints)
                    {
                        cv::circle(mask, pt, m_minDistance, cv::Scalar(0), -1);
                    }

                    cv::goodFeaturesToTrack(gray, newCorners, needed, m_qualityLevel,
                                           m_minDistance, mask);

                    newPoints.insert(newPoints.end(), newCorners.begin(), newCorners.end());
                }

                m_prevPoints = newPoints;
            }
        }
        else  // Farneback
        {
            // Farneback Dense Optical Flow
            cv::Mat flow;

            double pyrScale = 0.5;
            int levels = m_maxLevel;
            int winSize = m_windowSize;
            int iterations = 3;
            int polyN = 5;
            double polySigma = 1.1;
            int flags = 0;

            cv::calcOpticalFlowFarneback(m_prevGray, gray, flow, pyrScale, levels,
                                       winSize, iterations, polyN, polySigma, flags);

            // Draw flow field
            if (m_drawFlow)
            {
                // Draw flow vectors (subsample for clarity)
                int step = 16;  // Draw every 16th vector
                for (int y = 0; y < flow.rows; y += step)
                {
                    for (int x = 0; x < flow.cols; x += step)
                    {
                        cv::Point2f fxy = flow.at<cv::Point2f>(y, x);

                        // Draw arrow representing flow
                        cv::Point2f start(x, y);
                        cv::Point2f end(x + fxy.x * 3, y + fxy.y * 3);  // Scale for visibility

                        // Color based on magnitude
                        double magnitude = cv::sqrt(fxy.x * fxy.x + fxy.y * fxy.y);
                        cv::Scalar color;
                        if (magnitude < 1.0)
                        {
                            color = cv::Scalar(255, 0, 0);  // Blue (slow)
                        }
                        else if (magnitude < 3.0)
                        {
                            color = cv::Scalar(0, 255, 0);  // Green (medium)
                        }
                        else
                        {
                            color = cv::Scalar(0, 0, 255);  // Red (fast)
                        }

                        cv::line(output, start, end, color, 1);
                        cv::circle(output, end, 2, color, -1);
                    }
                }
            }
        }

        // Store current frame for next iteration
        gray.copyTo(m_prevGray);

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

} // namespace VisionBox
