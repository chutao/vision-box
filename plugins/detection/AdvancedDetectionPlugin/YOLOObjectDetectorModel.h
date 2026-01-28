/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * YOLO Object Detector Model Header
 ******************************************************************************/

#ifndef VISIONBOX_YOLOOBJECTDETECTORMODEL_H
#define VISIONBOX_YOLOOBJECTDETECTORMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * YOLOObjectDetectorModel - Real-time object detection with YOLO
 ******************************************************************************/
class YOLOObjectDetectorModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    YOLOObjectDetectorModel();
    ~YOLOObjectDetectorModel() override = default;

    QString caption() const override { return "YOLO Detector"; }
    QString name() const override { return "YOLOObjectDetectorModel"; }

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
    void onLoadModelClicked();
    void onClassesFileClicked();
    void onConfidenceChanged(double value);
    void onNmsThresholdChanged(double value);
    void onInputSizeChanged(int index);
    void onShowBoxesChanged(int state);
    void onShowLabelsChanged(int state);
    void onBackendChanged(int index);

private:
    void loadModel();
    void loadClasses();
    void runInference();
    cv::Mat preprocessImage(const cv::Mat& image);
    void postprocessAndDraw(cv::Mat& image, const std::vector<cv::Mat>& outputs);
    std::vector<int> getOutputLayers(const cv::dnn::Net& net);
    cv::Mat getOutputBlob(const std::vector<cv::Mat>& outputs);

private:
    // Model parameters
    QString m_modelPath;           // Path to YOLO weights/model
    QString m_configPath;          // Path to YOLO config (for YOLOv3/v4)
    QString m_classesPath;         // Path to classes names file
    int m_yoloVersion = 0;         // 0=YOLOv3, 1=YOLOv4, 2=YOLOv5/ONNX
    int m_backendIndex = 0;        // 0=CPU, 1=CUDA

    // Detection parameters
    double m_confidenceThreshold = 0.5;   // Confidence threshold (0.0-1.0)
    double m_nmsThreshold = 0.4;          // Non-maximum suppression (0.0-1.0)
    int m_inputSizeIndex = 0;             // Input size selection
    float m_inputScale = 1.0 / 255.0;     // Scale factor
    cv::Scalar m_mean = {0, 0, 0};        // Mean subtraction
    bool m_swapRB = true;                 // Swap Red and Blue

    // Visualization
    bool m_showBoxes = true;        // Show bounding boxes
    bool m_showLabels = true;       // Show class labels
    bool m_showConfidence = true;   // Show confidence scores

    // Detection results
    struct Detection
    {
        int classId;
        float confidence;
        cv::Rect box;
    };
    std::vector<Detection> m_detections;
    std::vector<std::string> m_classNames;

    // Network
    cv::Ptr<cv::dnn::Net> m_net;
    bool m_modelLoaded = false;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    cv::Mat m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QLineEdit* m_modelEdit = nullptr;
    QPushButton* m_modelBtn = nullptr;
    QLineEdit* m_configEdit = nullptr;
    QPushButton* m_configBtn = nullptr;
    QLineEdit* m_classesEdit = nullptr;
    QPushButton* m_classesBtn = nullptr;
    QComboBox* m_yoloVersionCombo = nullptr;
    QComboBox* m_inputSizeCombo = nullptr;
    QComboBox* m_backendCombo = nullptr;
    QDoubleSpinBox* m_confidenceSpin = nullptr;
    QDoubleSpinBox* m_nmsSpin = nullptr;
    QCheckBox* m_showBoxesCheck = nullptr;
    QCheckBox* m_showLabelsCheck = nullptr;
    QPushButton* m_loadBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
    QTextEdit* m_infoText = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_YOLOOBJECTDETECTORMODEL_H
