/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Watershed Segmentation Model Implementation
 ******************************************************************************/

#include "WatershedSegmentationModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
WatershedSegmentationModel::WatershedSegmentationModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Number of markers
    auto* markersLayout = new QHBoxLayout();
    markersLayout->addWidget(new QLabel("Auto Markers:"));
    m_markersSpin = new QSpinBox();
    m_markersSpin->setRange(2, 50);
    m_markersSpin->setValue(8);
    markersLayout->addWidget(m_markersSpin);
    layout->addLayout(markersLayout);

    // Iterations for distance transform
    auto* iterLayout = new QHBoxLayout();
    iterLayout->addWidget(new QLabel("Iterations:"));
    m_iterationsSpin = new QSpinBox();
    m_iterationsSpin->setRange(1, 20);
    m_iterationsSpin->setValue(5);
    iterLayout->addWidget(m_iterationsSpin);
    layout->addLayout(iterLayout);

    // Color regions
    m_colorRegionsCheck = new QCheckBox("Color Regions");
    m_colorRegionsCheck->setChecked(true);
    layout->addWidget(m_colorRegionsCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_markersSpin, &QSpinBox::valueChanged,
            this, &WatershedSegmentationModel::onMarkersChanged);
    connect(m_iterationsSpin, &QSpinBox::valueChanged,
            this, &WatershedSegmentationModel::onIterationsChanged);
    connect(m_colorRegionsCheck, &QCheckBox::stateChanged,
            this, &WatershedSegmentationModel::onColorRegionsChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int WatershedSegmentationModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType WatershedSegmentationModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> WatershedSegmentationModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void WatershedSegmentationModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                        QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    performWatershed();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* WatershedSegmentationModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void WatershedSegmentationModel::onMarkersChanged(int value)
{
    m_markers = value;
    performWatershed();
}

void WatershedSegmentationModel::onIterationsChanged(int value)
{
    m_iterations = value;
    performWatershed();
}

void WatershedSegmentationModel::onColorRegionsChanged(int state)
{
    m_colorRegions = (state == Qt::Checked);
    performWatershed();
}

/*******************************************************************************
 * Watershed Segmentation
 ******************************************************************************/
void WatershedSegmentationModel::performWatershed()
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

        // Apply threshold to get binary image
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        // Compute distance transform
        cv::Mat dist;
        cv::distanceTransform(binary, dist, cv::DIST_L2, 5);

        // Normalize distance transform
        cv::normalize(dist, dist, 0, 255, cv::NORM_MINMAX);

        // Create markers for watershed
        cv::Mat markers = cv::Mat::zeros(dist.size(), CV_32S);

        // Threshold to get sure foreground
        cv::Mat sureForeground;
        cv::threshold(dist, sureForeground, 200, 255, cv::THRESH_BINARY);

        // Threshold to get sure background
        cv::Mat sureBackground;
        cv::threshold(dist, sureBackground, 100, 255, cv::THRESH_BINARY_INV);

        // Find unknown region
        cv::Mat unknown;
        cv::subtract(sureBackground, sureForeground, unknown);

        // Create markers (label the regions)
        int labelCount = 0;
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(sureForeground, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Process contours to create markers
        for (size_t i = 0; i < contours.size(); ++i)
        {
            cv::drawContours(markers, contours, static_cast<int>(i),
                           cv::Scalar::all(++labelCount), -1);
        }

        // Add remaining markers if needed
        if (labelCount < m_markers)
        {
            // Use morphological operations to create more markers
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
            cv::dilate(sureForeground, sureForeground, kernel, cv::Point(-1, -1), m_iterations);

            // Find contours again
            contours.clear();
            cv::findContours(sureForeground, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            for (size_t i = 0; i < contours.size(); ++i)
            {
                if (labelCount >= m_markers)
                    break;

                // Only add if not already marked
                cv::Mat mask = cv::Mat::zeros(markers.size(), CV_8UC1);
                cv::drawContours(mask, contours, static_cast<int>(i),
                               cv::Scalar(255), -1);

                if (cv::countNonZero(markers & (mask > 0)) == 0)
                {
                    cv::drawContours(markers, contours, static_cast<int>(i),
                                   cv::Scalar::all(++labelCount), -1);
                }
            }
        }

        // Mark background as 1
        markers.setTo(1, sureBackground == 255);

        // Perform watershed
        cv::Mat result;
        if (input.channels() == 1)
        {
            cv::cvtColor(input, result, cv::COLOR_GRAY2BGR);
        }
        else
        {
            result = input.clone();
        }

        // Convert markers to proper format
        cv::Mat markers8u;
        markers.convertTo(markers8u, CV_8U);

        // Apply watershed
        cv::watershed(result, markers8u);

        // Color the regions
        if (m_colorRegions)
        {
            // Create random colors for each region
            std::vector<cv::Vec3b> colors;
            colors.push_back(cv::Vec3b(0, 0, 0));  // Background

            for (int i = 1; i <= m_markers; ++i)
            {
                colors.push_back(cv::Vec3b(rand() % 256, rand() % 256, rand() % 256));
            }

            // Color each region
            for (int y = 0; y < markers8u.rows; ++y)
            {
                for (int x = 0; x < markers8u.cols; ++x)
                {
                    int label = markers8u.at<uchar>(y, x);
                    if (label == -1)
                    {
                        // Boundary - draw in white
                        result.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
                    }
                    else if (label > 0 && label <= m_markers)
                    {
                        // Region - color it
                        result.at<cv::Vec3b>(y, x) = colors[label];
                    }
                }
            }
        }
        else
        {
            // Just draw boundaries
            for (int y = 0; y < markers8u.rows; ++y)
            {
                for (int x = 0; x < markers8u.cols; ++x)
                {
                    int label = markers8u.at<uchar>(y, x);
                    if (label == -1)
                    {
                        // Boundary
                        result.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
                    }
                }
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

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject WatershedSegmentationModel::save() const
{
    QJsonObject modelJson;
    modelJson["markers"] = m_markers;
    modelJson["iterations"] = m_iterations;
    modelJson["colorRegions"] = m_colorRegions;
    return modelJson;
}

void WatershedSegmentationModel::load(QJsonObject const& model)
{
    QJsonValue markersJson = model["markers"];
    if (!markersJson.isUndefined())
    {
        m_markers = markersJson.toInt();
        m_markersSpin->setValue(m_markers);
    }

    QJsonValue iterJson = model["iterations"];
    if (!iterJson.isUndefined())
    {
        m_iterations = iterJson.toInt();
        m_iterationsSpin->setValue(m_iterations);
    }

    QJsonValue colorJson = model["colorRegions"];
    if (!colorJson.isUndefined())
    {
        m_colorRegions = colorJson.toBool();
        m_colorRegionsCheck->setChecked(m_colorRegions);
    }

    performWatershed();
}

} // namespace VisionBox
