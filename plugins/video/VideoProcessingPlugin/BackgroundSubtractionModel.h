/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Background Subtraction Model Header
 ******************************************************************************/

#ifndef BACKGROUNDSUBTRACTIONMODEL_H
#define BACKGROUNDSUBTRACTIONMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

namespace VisionBox {

class ImageData;

class BackgroundSubtractionModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum class Algorithm
    {
        MOG2,
        KNN
    };

    explicit BackgroundSubtractionModel();
    ~BackgroundSubtractionModel() override = default;

    QString caption() const override { return "Background Subtraction"; }
    QString name() const override { return "BackgroundSubtractionModel"; }

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
    void onAlgorithmChanged(int index);
    void onHistoryChanged(int value);
    void onThresholdChanged(double value);
    void onDetectShadowsChanged(int state);
    void onLearningRateChanged(double value);
    void onResetClicked();

private:
    void applyBackgroundSubtraction();
    void resetBackground();

private:
    // Parameters
    Algorithm m_algorithm = Algorithm::MOG2;
    int m_history = 500;
    double m_threshold = 16.0;
    bool m_detectShadows = true;
    double m_learningRate = -1.0;  // -1 = auto

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // Background subtractors
    cv::Ptr<cv::BackgroundSubtractorMOG2> m_mog2;
    cv::Ptr<cv::BackgroundSubtractorKNN> m_knn;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_algorithmCombo = nullptr;
    QSpinBox* m_historySpin = nullptr;
    QDoubleSpinBox* m_thresholdSpin = nullptr;
    QCheckBox* m_detectShadowsCheck = nullptr;
    QDoubleSpinBox* m_learningRateSpin = nullptr;
    QPushButton* m_resetButton = nullptr;
};

} // namespace VisionBox

#endif // BACKGROUNDSUBTRACTIONMODEL_H
