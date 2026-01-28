/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Core Vision Data Types for Node Graph
 ******************************************************************************/

#ifndef VISIONBOX_VISIONDATATYPES_H
#define VISIONBOX_VISIONDATATYPES_H

#include <QtNodes/NodeData>
#include <opencv2/core/mat.hpp>
#include <opencv2/features2d.hpp>
#include <QImage>
#include <QVector>
#include <QRectF>
#include <QString>
#include <memory>

namespace VisionBox {

/*******************************************************************************
 * ImageData - Wraps OpenCV cv::Mat for node data flow
 ******************************************************************************/
class ImageData : public QtNodes::NodeData
{
public:
    ImageData() = default;

    explicit ImageData(const cv::Mat& image)
        : m_image(image)
    {
    }

    // Get the OpenCV Mat
    cv::Mat image() const
    {
        return m_image;
    }

    // Set the image
    void setImage(const cv::Mat& image)
    {
        m_image = image;
    }

    // Convert to QImage for display
    QImage toQImage() const;

    // NodeData interface
    QtNodes::NodeDataType type() const override
    {
        return QtNodes::NodeDataType{"opencv_image", "Image"};
    }

    // Check if image is valid
    bool isValid() const
    {
        return !m_image.empty();
    }

    // Get image dimensions
    int width() const
    {
        return m_image.cols;
    }

    int height() const
    {
        return m_image.rows;
    }

    int channels() const
    {
        return m_image.channels();
    }

private:
    cv::Mat m_image;
};

/*******************************************************************************
 * DetectionData - Represents object detection results
 ******************************************************************************/
class DetectionData : public ::QtNodes::NodeData
{
public:
    // Single detection result
    struct Detection
    {
        QRectF bbox;        // Bounding box (normalized coordinates [0,1])
        QString label;      // Class label
        float confidence;   // Confidence score [0,1]

        Detection()
            : confidence(0.0f)
        {
        }

        Detection(const QRectF& b, const QString& l, float c)
            : bbox(b)
            , label(l)
            , confidence(c)
        {
        }
    };

    DetectionData() = default;

    explicit DetectionData(const QVector<Detection>& detections)
        : m_detections(detections)
    {
    }

    // Get all detections
    QVector<Detection> detections() const
    {
        return m_detections;
    }

    // Add a detection
    void addDetection(const Detection& detection)
    {
        m_detections.append(detection);
    }

    void addDetection(const QRectF& bbox, const QString& label, float confidence)
    {
        m_detections.append(Detection(bbox, label, confidence));
    }

    // Clear all detections
    void clear()
    {
        m_detections.clear();
    }

    // Get count
    int count() const
    {
        return m_detections.size();
    }

    bool isEmpty() const
    {
        return m_detections.isEmpty();
    }

    // NodeData interface
    ::QtNodes::NodeDataType type() const override
    {
        return ::QtNodes::NodeDataType{"detection", "Detection"};
    }

private:
    QVector<Detection> m_detections;
};

/*******************************************************************************
 * KeypointData - Represents feature keypoints
 ******************************************************************************/
class KeypointData : public ::QtNodes::NodeData
{
public:
    KeypointData() = default;

    explicit KeypointData(const std::vector<cv::KeyPoint>& keypoints)
        : m_keypoints(keypoints)
    {
    }

    // Get all keypoints
    std::vector<cv::KeyPoint> keypoints() const
    {
        return m_keypoints;
    }

    // Set keypoints
    void setKeypoints(const std::vector<cv::KeyPoint>& keypoints)
    {
        m_keypoints = keypoints;
    }

    // Add a keypoint
    void addKeypoint(const cv::KeyPoint& keypoint)
    {
        m_keypoints.push_back(keypoint);
    }

    // Clear all keypoints
    void clear()
    {
        m_keypoints.clear();
    }

    // Get count
    int count() const
    {
        return static_cast<int>(m_keypoints.size());
    }

    bool isEmpty() const
    {
        return m_keypoints.empty();
    }

    // NodeData interface
    ::QtNodes::NodeDataType type() const override
    {
        return ::QtNodes::NodeDataType{"keypoints", "Keypoints"};
    }

private:
    std::vector<cv::KeyPoint> m_keypoints;
};

} // namespace VisionBox

#endif // VISIONBOX_VISIONDATATYPES_H
