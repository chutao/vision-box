/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * ORB Feature Model Header
 ******************************************************************************/

#ifndef ORBFEATUREMODEL_H
#define ORBFEATUREMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>

namespace VisionBox {

class ImageData;

class ORBFeatureModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    explicit ORBFeatureModel();
    ~ORBFeatureModel() override = default;

    QString caption() const override { return "ORB Features"; }
    QString name() const override { return "ORBFeatureModel"; }

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
    void onMaxFeaturesChanged(int value);
    void onScaleFactorChanged(double value);
    void onNLevelsChanged(int value);
    void onEdgeThresholdChanged(int value);
    void onFirstLevelChanged(int value);
    void onWTAKChanged(int value);
    void onScoreTypeChanged(int index);
    void onPatchSizeChanged(int value);
    void onFastThresholdChanged(int value);
    void onDrawKeypointsChanged(int state);
    void onDrawRichChanged(int state);

private:
    void detectFeatures();

private:
    // Parameters
    int m_maxFeatures = 500;
    double m_scaleFactor = 1.2;
    int m_nLevels = 8;
    int m_edgeThreshold = 31;
    int m_firstLevel = 0;
    int m_wtaK = 2;
    int m_scoreType = 0;  // 0 = HARRIS_SCORE, 1 = FAST_SCORE
    int m_patchSize = 31;
    int m_fastThreshold = 20;
    bool m_drawKeypoints = true;
    bool m_drawRich = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QSpinBox* m_maxFeaturesSpin = nullptr;
    QDoubleSpinBox* m_scaleFactorSpin = nullptr;
    QSpinBox* m_nLevelsSpin = nullptr;
    QSpinBox* m_edgeThresholdSpin = nullptr;
    QSpinBox* m_firstLevelSpin = nullptr;
    QSpinBox* m_wtaKSpin = nullptr;
    QComboBox* m_scoreTypeCombo = nullptr;
    QSpinBox* m_patchSizeSpin = nullptr;
    QSpinBox* m_fastThresholdSpin = nullptr;
    QCheckBox* m_drawKeypointsCheck = nullptr;
    QCheckBox* m_drawRichCheck = nullptr;
};

} // namespace VisionBox

#endif // ORBFEATUREMODEL_H
