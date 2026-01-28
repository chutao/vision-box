/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * SIFT Features Model Header
 ******************************************************************************/

#ifndef VISIONBOX_SIFTFEATURESMODEL_H
#define VISIONBOX_SIFTFEATURESMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

namespace VisionBox {

class ImageData;
class KeypointData;

/*******************************************************************************
 * SIFTFeaturesModel - SIFT (Scale-Invariant Feature Transform) detector
 ******************************************************************************/
class SIFTFeaturesModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    SIFTFeaturesModel();
    ~SIFTFeaturesModel() override = default;

    QString caption() const override { return "SIFT Features"; }
    QString name() const override { return "SIFTFeaturesModel"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;

    QWidget* embeddedWidget() override;

    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

public slots:
    void onFeaturesChanged(int value);
    void onOctaveLayersChanged(int value);
    void onContrastThresholdChanged(double value);
    void onEdgeThresholdChanged(double value);
    void onSigmaChanged(double value);
    void onShowKeypointsChanged(int state);

private:
    void detectFeatures();

private:
    // SIFT parameters
    int m_nFeatures = 0;           // 0 = detect all features
    int m_nOctaveLayers = 3;       // Number of octave layers
    double m_contrastThreshold = 0.04;  // Contrast threshold
    double m_edgeThreshold = 10.0;      // Edge threshold
    double m_sigma = 1.6;          // Gaussian sigma

    // Visualization
    bool m_showKeypoints = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;  // Visualization
    std::shared_ptr<KeypointData> m_keypointData;

    // SIFT detector
    cv::Ptr<cv::SIFT> m_sift;

    // UI
    QWidget* m_widget = nullptr;
    QSpinBox* m_featuresSpin = nullptr;
    QSpinBox* m_octaveLayersSpin = nullptr;
    QDoubleSpinBox* m_contrastThresholdSpin = nullptr;
    QDoubleSpinBox* m_edgeThresholdSpin = nullptr;
    QDoubleSpinBox* m_sigmaSpin = nullptr;
    QCheckBox* m_showKeypointsCheck = nullptr;
    QLabel* m_keypointsLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_SIFTFEATURESMODEL_H
