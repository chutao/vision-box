/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * DNN Inference Model Header
 ******************************************************************************/

#ifndef VISIONBOX_DNNINFERENCEMODEL_H
#define VISIONBOX_DNNINFERENCEMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * DNNInferenceModel - Deep Neural Network inference
 ******************************************************************************/
class DNNInferenceModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    DNNInferenceModel();
    ~DNNInferenceModel() override;

    QString caption() const override { return "DNN Inference"; }
    QString name() const override { return "DNNInferenceModel"; }

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
    void onBrowseModel();
    void onBrowseConfig();
    void onBrowseClasses();
    void onLoadModel();
    void onBackendChanged();
    void onTargetChanged();
    void onConfidenceChanged(double value);
    void onSwapRBChanged(int state);
    void runInference();

private:
    void loadModelFiles();
    void preprocessImage(const cv::Mat& image, cv::Mat& blob, cv::Size& inputSize);
    cv::Mat visualizeDetections(const cv::Mat& image, const std::vector<cv::Mat>& outs);

private:
    // Model files
    QString m_modelPath;
    QString m_configPath;      // For Caffe/TensorFlow models
    QString m_classesPath;     // Class names file

    // Network configuration
    int m_backend = cv::dnn::DNN_BACKEND_OPENCV;
    int m_target = cv::dnn::DNN_TARGET_CPU;

    // Inference parameters
    double m_confidenceThreshold = 0.5;  // Confidence threshold for detections
    double m_nmsThreshold = 0.4;         // Non-maximum suppression threshold
    bool m_swapRB = true;                // Swap Red and Blue channels
    cv::Scalar m_mean = cv::Scalar(0, 0, 0, 0);
    double m_scale = 1.0;
    cv::Size m_inputSize = cv::Size(640, 640);

    // Network
    cv::Ptr<cv::dnn::Net> m_net;
    bool m_modelLoaded = false;

    // Class names
    std::vector<std::string> m_classes;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QLineEdit* m_modelPathEdit = nullptr;
    QPushButton* m_modelBrowseBtn = nullptr;
    QLineEdit* m_configPathEdit = nullptr;
    QPushButton* m_configBrowseBtn = nullptr;
    QLineEdit* m_classesPathEdit = nullptr;
    QPushButton* m_classesBrowseBtn = nullptr;
    QPushButton* m_loadBtn = nullptr;
    QComboBox* m_backendCombo = nullptr;
    QComboBox* m_targetCombo = nullptr;
    QDoubleSpinBox* m_confidenceSpin = nullptr;
    QDoubleSpinBox* m_nmsSpin = nullptr;
    QCheckBox* m_swapRBCheck = nullptr;
    QLabel* m_statusLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_DNNINFERENCEMODEL_H
