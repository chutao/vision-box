/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Object Tracker Model Header
 ******************************************************************************/

#ifndef VISIONBOX_OBJECTTRACKERMODEL_H
#define VISIONBOX_OBJECTTRACKERMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/video.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ObjectTrackerModel - Track objects in video sequences
 ******************************************************************************/
class ObjectTrackerModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    ObjectTrackerModel();
    ~ObjectTrackerModel() override;

    QString caption() const override { return "Object Tracker"; }
    QString name() const override { return "ObjectTrackerModel"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;

    QWidget* embeddedWidget() override;

    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

private slots:
    void onTrackerTypeChanged();
    void onThresholdChanged(double value);
    void onMorphologyChanged(int state);
    void onMinAreaChanged(int value);
    void onMaxAreaChanged(int value);
    void onShowBboxChanged(int state);
    void onResetClicked();

private:
    void processFrame();
    void initializeTracker();
    cv::Mat detectMotion();

private:
    enum TrackerType
    {
        MotionDetection = 0,     // Background subtraction
        DenseOpticalFlow,         // Farneback optical flow
        SparseOpticalFlow         // Pyramidal Lucas-Kanade
    };

    // Parameters
    TrackerType m_trackerType = MotionDetection;
    double m_threshold = 25.0;          // Motion detection threshold
    bool m_morphology = true;            // Apply morphological operations
    int m_minArea = 100;                 // Minimum object area
    int m_maxArea = 100000;              // Maximum object area
    bool m_showBbox = true;              // Show bounding boxes

    // Tracking state
    cv::Mat m_prevFrame;                 // Previous frame for optical flow
    cv::Mat m_background;                // Background model
    cv::Ptr<cv::BackgroundSubtractorMOG2> m_bgSubtractor;
    std::vector<cv::Point2f> m_prevPoints; // Previous feature points
    bool m_initialized = false;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_trackerTypeCombo = nullptr;
    QDoubleSpinBox* m_thresholdSpin = nullptr;
    QCheckBox* m_morphologyCheck = nullptr;
    QSpinBox* m_minAreaSpin = nullptr;
    QSpinBox* m_maxAreaSpin = nullptr;
    QCheckBox* m_showBboxCheck = nullptr;
    QPushButton* m_resetBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_OBJECTTRACKERMODEL_H
