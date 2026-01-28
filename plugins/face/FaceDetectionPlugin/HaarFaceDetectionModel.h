/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Haar Face Detection Model Header
 ******************************************************************************/

#ifndef HAARFACEDETECTIONMODEL_H
#define HAARFACEDETECTIONMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

namespace VisionBox {

class ImageData;

class HaarFaceDetectionModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum class DetectionType
    {
        Face,
        Eyes,
        Smile
    };

    explicit HaarFaceDetectionModel();
    ~HaarFaceDetectionModel() override = default;

    QString caption() const override { return "Haar Face Detection"; }
    QString name() const override { return "HaarFaceDetectionModel"; }

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
    void onDetectionTypeChanged(int index);
    void onScaleFactorChanged(double value);
    void onMinNeighborsChanged(int value);
    void onMinSizeChanged(int value);
    void onDrawBoxesChanged(int state);

private:
    void detectFaces();
    bool loadCascade(const std::string& cascadePath);

private:
    // Parameters
    DetectionType m_detectionType = DetectionType::Face;
    double m_scaleFactor = 1.1;
    int m_minNeighbors = 3;
    int m_minSize = 30;
    bool m_drawBoxes = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // Cascade classifiers
    cv::CascadeClassifier m_faceCascade;
    cv::CascadeClassifier m_eyeCascade;
    cv::CascadeClassifier m_smileCascade;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_detectionTypeCombo = nullptr;
    QDoubleSpinBox* m_scaleFactorSpin = nullptr;
    QSpinBox* m_minNeighborsSpin = nullptr;
    QSpinBox* m_minSizeSpin = nullptr;
    QCheckBox* m_drawBoxesCheck = nullptr;
};

} // namespace VisionBox

#endif // HAARFACEDETECTIONMODEL_H
