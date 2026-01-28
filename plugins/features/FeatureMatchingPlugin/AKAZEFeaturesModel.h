/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * AKAZE Features Model Header
 ******************************************************************************/

#ifndef VISIONBOX_AKAZEFEATURESMODEL_H
#define VISIONBOX_AKAZEFEATURESMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

namespace VisionBox {

class ImageData;
class KeypointData;

/*******************************************************************************
 * AKAZEFeaturesModel - AKAZE (Accelerated-KAZE) detector
 ******************************************************************************/
class AKAZEFeaturesModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    AKAZEFeaturesModel();
    ~AKAZEFeaturesModel() override = default;

    QString caption() const override { return "AKAZE Features"; }
    QString name() const override { return "AKAZEFeaturesModel"; }

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
    void onDescriptorTypeChanged();
    void onDescriptorSizeChanged(int value);
    void onDescriptorChannelsChanged(int value);
    void onThresholdChanged(double value);
    void onNOctavesChanged(int value);
    void onNOctaveLayersChanged(int value);
    void onDiffusivityChanged();
    void onShowKeypointsChanged(int state);

private:
    void detectFeatures();

private:
    // AKAZE parameters
    int m_descriptorType = cv::AKAZE::DESCRIPTOR_MLDB;  // Descriptor type
    int m_descriptorSize = 0;        // Descriptor size (0 = full size)
    int m_descriptorChannels = 3;     // Descriptor channels
    double m_threshold = 0.001;      // Detector response threshold
    int m_nOctaves = 4;              // Number of octaves
    int m_nOctaveLayers = 4;         // Number of octave layers
    int m_diffusivity = cv::KAZE::DIFF_PM_G2;  // Diffusivity

    // Visualization
    bool m_showKeypoints = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;  // Visualization
    std::shared_ptr<KeypointData> m_keypointData;

    // AKAZE detector
    cv::Ptr<cv::AKAZE> m_akaze;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_descriptorTypeCombo = nullptr;
    QSpinBox* m_descriptorSizeSpin = nullptr;
    QSpinBox* m_descriptorChannelsSpin = nullptr;
    QDoubleSpinBox* m_thresholdSpin = nullptr;
    QSpinBox* m_nOctavesSpin = nullptr;
    QSpinBox* m_nOctaveLayersSpin = nullptr;
    QComboBox* m_diffusivityCombo = nullptr;
    QCheckBox* m_showKeypointsCheck = nullptr;
    QLabel* m_keypointsLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_AKAZEFEATURESMODEL_H
