/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * HOG Detection Model Header
 ******************************************************************************/

#ifndef HOGDETECTIONMODEL_H
#define HOGDETECTIONMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

namespace VisionBox {

class ImageData;

class HOGDetectionModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    explicit HOGDetectionModel();
    ~HOGDetectionModel() override = default;

    QString caption() const override { return "HOG Detection"; }
    QString name() const override { return "HOGDetectionModel"; }

public:
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;
    QWidget* embeddedWidget() override;

    QJsonObject save() const;
    void load(QJsonObject const& model) override;

public slots:
    void onHitThresholdChanged(double value);
    void onWinStrideChanged(int value);
    void onPaddingChanged(int value);
    void onScaleChanged(double value);
    void onMeanShiftChanged(int state);
    void onDrawBoxesChanged(int state);

private:
    void detectObjects();

private:
    // Parameters
    double m_hitThreshold = 0.0;
    int m_winStride = 8;
    int m_padding = 8;
    double m_scale = 1.05;
    bool m_meanShift = false;
    bool m_drawBoxes = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // HOG descriptor
    cv::HOGDescriptor m_hog;

    // UI
    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_hitThresholdSpin = nullptr;
    QSpinBox* m_winStrideSpin = nullptr;
    QSpinBox* m_paddingSpin = nullptr;
    QDoubleSpinBox* m_scaleSpin = nullptr;
    QCheckBox* m_meanShiftCheck = nullptr;
    QCheckBox* m_drawBoxesCheck = nullptr;
};

} // namespace VisionBox

#endif // HOGDETECTIONMODEL_H
