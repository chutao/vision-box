/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Matcher Model Header
 ******************************************************************************/

#ifndef VISIONBOX_FEATUREMATCHERMODEL_H
#define VISIONBOX_FEATUREMATCHERMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

namespace VisionBox {

class ImageData;
class KeypointData;

/*******************************************************************************
 * FeatureMatcherModel - Match features between two images
 ******************************************************************************/
class FeatureMatcherModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    FeatureMatcherModel();
    ~FeatureMatcherModel() override = default;

    QString caption() const override { return "Feature Matcher"; }
    QString name() const override { return "FeatureMatcherModel"; }

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
    void onMatcherChanged();
    void onNormTypeChanged();
    void onCrossCheckChanged(int state);
    void onMaxRatioChanged(double value);
    void onMaxDistanceChanged(double value);
    void onMaxMatchesChanged(int value);

private:
    void matchFeatures();
    cv::Ptr<cv::DescriptorMatcher> createMatcher();

private:
    enum MatcherType
    {
        BruteForce = 0,
        BruteForceSL2,
        BruteForceL1,
        BruteForceHamming,
        BruteForceHammingLUT,
        FlannBased
    };

    // Parameters
    MatcherType m_matcherType = FlannBased;
    int m_normType = cv::NORM_L2;      // For BruteForce variants
    bool m_crossCheck = false;         // Cross-check filtering
    double m_maxRatio = 0.75;          // Ratio test threshold (Lowe's ratio test)
    double m_maxDistance = 100.0;      // Maximum distance threshold
    int m_maxMatches = 100;            // Maximum number of matches to display

    // Data
    std::shared_ptr<ImageData> m_inputImage1;      // First image
    std::shared_ptr<ImageData> m_inputImage2;      // Second image
    std::shared_ptr<KeypointData> m_keypoints1;    // First keypoints
    std::shared_ptr<KeypointData> m_keypoints2;    // Second keypoints
    std::shared_ptr<ImageData> m_outputImage;      // Match visualization

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_matcherCombo = nullptr;
    QComboBox* m_normTypeCombo = nullptr;
    QCheckBox* m_crossCheckCheck = nullptr;
    QDoubleSpinBox* m_maxRatioSpin = nullptr;
    QDoubleSpinBox* m_maxDistanceSpin = nullptr;
    QSpinBox* m_maxMatchesSpin = nullptr;
    QLabel* m_matchesLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_FEATUREMATCHERMODEL_H
