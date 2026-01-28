/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Optical Flow Model Header
 ******************************************************************************/

#ifndef OPTICALFLOWMODEL_H
#define OPTICALFLOWMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

namespace VisionBox {

class ImageData;

class OpticalFlowModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum class FlowMethod
    {
        LucasKanade,
        Farneback
    };

    explicit OpticalFlowModel();
    ~OpticalFlowModel() override = default;

    QString caption() const override { return "Optical Flow"; }
    QString name() const override { return "OpticalFlowModel"; }

public:
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;
    QWidget* embeddedWidget() override;

public slots:
    void onMethodChanged(int index);
    void onMaxCornersChanged(int value);
    void onQualityLevelChanged(double value);
    void onMinDistanceChanged(double value);
    void onWindowSizeChanged(int value);
    void onMaxLevelChanged(int value);
    void onDrawFlowChanged(int state);

private:
    void computeFlow();

private:
    // Parameters
    FlowMethod m_method = FlowMethod::LucasKanade;
    int m_maxCorners = 100;
    double m_qualityLevel = 0.01;
    double m_minDistance = 10.0;
    int m_windowSize = 15;
    int m_maxLevel = 3;
    bool m_drawFlow = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    cv::Mat m_prevGray;
    std::vector<cv::Point2f> m_prevPoints;
    bool m_hasPreviousFrame = false;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_methodCombo = nullptr;
    QSpinBox* m_maxCornersSpin = nullptr;
    QDoubleSpinBox* m_qualityLevelSpin = nullptr;
    QDoubleSpinBox* m_minDistanceSpin = nullptr;
    QSpinBox* m_windowSizeSpin = nullptr;
    QSpinBox* m_maxLevelSpin = nullptr;
    QCheckBox* m_drawFlowCheck = nullptr;
};

} // namespace VisionBox

#endif // OPTICALFLOWMODEL_H
