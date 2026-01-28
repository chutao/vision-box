/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * YOLO Object Detector Model Implementation
 ******************************************************************************/

#include "YOLOObjectDetectorModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <algorithm>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
YOLOObjectDetectorModel::YOLOObjectDetectorModel()
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // YOLO version selection
    auto* versionLayout = new QHBoxLayout();
    versionLayout->addWidget(new QLabel("YOLO Version:"));
    m_yoloVersionCombo = new QComboBox();
    m_yoloVersionCombo->addItem("YOLOv3 (Darknet)", 0);
    m_yoloVersionCombo->addItem("YOLOv4 (Darknet)", 1);
    m_yoloVersionCombo->addItem("YOLOv5/8 (ONNX)", 2);
    m_yoloVersionCombo->setCurrentIndex(2);  // Default to ONNX
    m_yoloVersionCombo->setMinimumWidth(150);
    versionLayout->addWidget(m_yoloVersionCombo);
    layout->addLayout(versionLayout);

    // Model file
    auto* modelLayout = new QHBoxLayout();
    modelLayout->addWidget(new QLabel("Model:"));
    m_modelEdit = new QLineEdit();
    m_modelEdit->setPlaceholderText("/path/to/model.onnx or yolov3.weights");
    modelLayout->addWidget(m_modelEdit);
    m_modelBtn = new QPushButton("Browse...");
    modelLayout->addWidget(m_modelBtn);
    layout->addLayout(modelLayout);

    // Config file (for YOLOv3/v4)
    auto* configLayout = new QHBoxLayout();
    configLayout->addWidget(new QLabel("Config:"));
    m_configEdit = new QLineEdit();
    m_configEdit->setPlaceholderText("/path/to/yolov3.cfg (for Darknet)");
    configLayout->addWidget(m_configEdit);
    m_configBtn = new QPushButton("Browse...");
    configLayout->addWidget(m_configBtn);
    layout->addLayout(configLayout);

    // Classes file
    auto* classesLayout = new QHBoxLayout();
    classesLayout->addWidget(new QLabel("Classes:"));
    m_classesEdit = new QLineEdit();
    m_classesEdit->setPlaceholderText("/path/to/coco.names");
    classesLayout->addWidget(m_classesEdit);
    m_classesBtn = new QPushButton("Browse...");
    classesLayout->addWidget(m_classesBtn);
    layout->addLayout(classesLayout);

    // Input size
    auto* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Input Size:"));
    m_inputSizeCombo = new QComboBox();
    m_inputSizeCombo->addItem("320x320", 0);
    m_inputSizeCombo->addItem("416x416", 1);
    m_inputSizeCombo->addItem("512x512", 2);
    m_inputSizeCombo->addItem("608x608", 3);
    m_inputSizeCombo->addItem("640x640", 4);
    m_inputSizeCombo->setCurrentIndex(4);  // Default 640x640
    m_inputSizeCombo->setMinimumWidth(150);
    sizeLayout->addWidget(m_inputSizeCombo);
    layout->addLayout(sizeLayout);

    // Backend selection
    auto* backendLayout = new QHBoxLayout();
    backendLayout->addWidget(new QLabel("Backend:"));
    m_backendCombo = new QComboBox();
    m_backendCombo->addItem("CPU", 0);
    m_backendCombo->addItem("CUDA (GPU)", 1);
    m_backendCombo->setCurrentIndex(0);
    m_backendCombo->setMinimumWidth(150);
    backendLayout->addWidget(m_backendCombo);
    layout->addLayout(backendLayout);

    // Confidence threshold
    auto* confLayout = new QHBoxLayout();
    confLayout->addWidget(new QLabel("Confidence:"));
    m_confidenceSpin = new QDoubleSpinBox();
    m_confidenceSpin->setRange(0.0, 1.0);
    m_confidenceSpin->setSingleStep(0.05);
    m_confidenceSpin->setValue(0.5);
    confLayout->addWidget(m_confidenceSpin);
    layout->addLayout(confLayout);

    // NMS threshold
    auto* nmsLayout = new QHBoxLayout();
    nmsLayout->addWidget(new QLabel("NMS Threshold:"));
    m_nmsSpin = new QDoubleSpinBox();
    m_nmsSpin->setRange(0.0, 1.0);
    m_nmsSpin->setSingleStep(0.05);
    m_nmsSpin->setValue(0.4);
    nmsLayout->addWidget(m_nmsSpin);
    layout->addLayout(nmsLayout);

    // Visualization options
    m_showBoxesCheck = new QCheckBox("Show Bounding Boxes");
    m_showBoxesCheck->setChecked(true);
    layout->addWidget(m_showBoxesCheck);

    m_showLabelsCheck = new QCheckBox("Show Labels & Confidence");
    m_showLabelsCheck->setChecked(true);
    layout->addWidget(m_showLabelsCheck);

    // Load button
    m_loadBtn = new QPushButton("Load Model");
    m_loadBtn->setEnabled(false);
    layout->addWidget(m_loadBtn);

    // Status label
    m_statusLabel = new QLabel("Status: No model loaded");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");
    layout->addWidget(m_statusLabel);

    // Info text
    m_infoText = new QTextEdit();
    m_infoText->setReadOnly(true);
    m_infoText->setMaximumHeight(80);
    m_infoText->setPlaceholderText("Detection info will appear here...");
    layout->addWidget(m_infoText);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_modelBtn, &QPushButton::clicked,
            this, &YOLOObjectDetectorModel::onLoadModelClicked);
    connect(m_classesBtn, &QPushButton::clicked,
            this, &YOLOObjectDetectorModel::onClassesFileClicked);
    connect(m_configBtn, &QPushButton::clicked,
            this, &YOLOObjectDetectorModel::onLoadModelClicked);
    connect(m_loadBtn, &QPushButton::clicked,
            this, &YOLOObjectDetectorModel::loadModel);
    connect(m_confidenceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &YOLOObjectDetectorModel::onConfidenceChanged);
    connect(m_nmsSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &YOLOObjectDetectorModel::onNmsThresholdChanged);
    connect(m_inputSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &YOLOObjectDetectorModel::onInputSizeChanged);
    connect(m_showBoxesCheck, &QCheckBox::stateChanged,
            this, &YOLOObjectDetectorModel::onShowBoxesChanged);
    connect(m_showLabelsCheck, &QCheckBox::stateChanged,
            this, &YOLOObjectDetectorModel::onShowLabelsChanged);
    connect(m_backendCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &YOLOObjectDetectorModel::onBackendChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int YOLOObjectDetectorModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image
    }
    else
    {
        return 1; // One output image (with detections)
    }
}

QtNodes::NodeDataType YOLOObjectDetectorModel::dataType(
    QtNodes::PortType portType,
    QtNodes::PortIndex portIndex) const
{
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    return ImageData().type();
}

/*******************************************************************************
 * Data Flow
 ******************************************************************************/
std::shared_ptr<QtNodes::NodeData> YOLOObjectDetectorModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return std::make_shared<ImageData>(m_outputImage);
}

void YOLOObjectDetectorModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                       QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    // Update load button state
    bool hasModel = !m_modelPath.isEmpty();
    if (m_yoloVersionCombo->currentData().toInt() < 2)
    {
        hasModel = hasModel && !m_configPath.isEmpty();
    }
    m_loadBtn->setEnabled(hasModel);

    // Run inference if model is loaded
    if (m_modelLoaded && m_inputImage)
    {
        runInference();
    }
    else if (m_inputImage)
    {
        // Just pass through the input
        m_outputImage = m_inputImage->image().clone();
    }

    Q_EMIT dataUpdated(0);
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* YOLOObjectDetectorModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void YOLOObjectDetectorModel::onLoadModelClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString filter;
    if (btn == m_modelBtn)
    {
        int version = m_yoloVersionCombo->currentData().toInt();
        if (version == 2)
        {
            filter = "ONNX Models (*.onnx);;All Files (*.*)";
        }
        else
        {
            filter = "Darknet Weights (*.weights);;All Files (*.*)";
        }

        QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            "Select YOLO Model",
            m_modelPath.isEmpty() ? QDir::homePath() : m_modelPath,
            filter
        );

        if (!filePath.isEmpty())
        {
            m_modelPath = filePath;
            m_modelEdit->setText(filePath);
        }
    }
    else if (btn == m_configBtn)
    {
        QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            "Select YOLO Config",
            m_configPath.isEmpty() ? QDir::homePath() : m_configPath,
            "Config Files (*.cfg *.txt);;All Files (*.*)"
        );

        if (!filePath.isEmpty())
        {
            m_configPath = filePath;
            m_configEdit->setText(filePath);
        }
    }

    // Update load button state
    bool hasModel = !m_modelPath.isEmpty();
    if (m_yoloVersionCombo->currentData().toInt() < 2)
    {
        hasModel = hasModel && !m_configPath.isEmpty();
    }
    m_loadBtn->setEnabled(hasModel);
}

void YOLOObjectDetectorModel::onClassesFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Select Classes File",
        m_classesPath.isEmpty() ? QDir::homePath() : m_classesPath,
        "Text Files (*.names *.txt);;All Files (*.*)"
    );

    if (!filePath.isEmpty())
    {
        m_classesPath = filePath;
        m_classesEdit->setText(filePath);
        loadClasses();
    }
}

void YOLOObjectDetectorModel::onConfidenceChanged(double value)
{
    m_confidenceThreshold = value;
}

void YOLOObjectDetectorModel::onNmsThresholdChanged(double value)
{
    m_nmsThreshold = value;
}

void YOLOObjectDetectorModel::onInputSizeChanged(int index)
{
    m_inputSizeIndex = index;
}

void YOLOObjectDetectorModel::onShowBoxesChanged(int state)
{
    m_showBoxes = (state == Qt::Checked);
}

void YOLOObjectDetectorModel::onShowLabelsChanged(int state)
{
    m_showLabels = (state == Qt::Checked);
}

void YOLOObjectDetectorModel::onBackendChanged(int index)
{
    m_backendIndex = index;
}

/*******************************************************************************
 * Model Loading
 ******************************************************************************/
void YOLOObjectDetectorModel::loadModel()
{
    if (m_modelPath.isEmpty())
    {
        m_statusLabel->setText("Status: No model file selected");
        return;
    }

    try
    {
        int version = m_yoloVersionCombo->currentData().toInt();

        // Load network based on version
        if (version == 2)
        {
            // YOLOv5/ONNX
            cv::dnn::Net net = cv::dnn::readNetFromONNX(m_modelPath.toStdString());
            m_net = cv::makePtr<cv::dnn::Net>(net);
        }
        else
        {
            // YOLOv3/v4 Darknet
            if (m_configPath.isEmpty())
            {
                m_statusLabel->setText("Status: Config file required for Darknet models");
                return;
            }

            cv::dnn::Net net = cv::dnn::readNetFromDarknet(
                m_configPath.toStdString(),
                m_modelPath.toStdString()
            );
            m_net = cv::makePtr<cv::dnn::Net>(net);
        }

        // Set backend and target
        if (m_backendIndex == 1)
        {
            // CUDA
            m_net->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            m_net->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
        }
        else
        {
            // CPU
            m_net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            m_net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        }

        m_modelLoaded = true;
        m_statusLabel->setText("Status: Model loaded successfully");

        // Load classes if file selected
        if (!m_classesPath.isEmpty())
        {
            loadClasses();
        }

        // Run inference on current image
        if (m_inputImage)
        {
            runInference();
        }
    }
    catch (const cv::Exception& e)
    {
        m_statusLabel->setText(QString("Status: Error loading model - %1").arg(e.what()));
        m_modelLoaded = false;
    }
}

void YOLOObjectDetectorModel::loadClasses()
{
    m_classNames.clear();

    QFile file(m_classesPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (!line.trimmed().isEmpty())
            {
                m_classNames.push_back(line.trimmed().toStdString());
            }
        }
        file.close();

        m_infoText->setText(QString("Loaded %1 class names").arg(m_classNames.size()));
    }
}

/*******************************************************************************
 * Inference
 ******************************************************************************/
void YOLOObjectDetectorModel::runInference()
{
    if (!m_modelLoaded || !m_net || !m_inputImage)
    {
        return;
    }

    cv::Mat image = m_inputImage->image();
    if (image.empty())
    {
        return;
    }

    // Preprocess
    cv::Mat blob = preprocessImage(image);

    // Set input
    m_net->setInput(blob);

    // Forward pass
    std::vector<cv::Mat> outputs;
    m_net->forward(outputs);

    // Postprocess and draw
    m_outputImage = image.clone();
    postprocessAndDraw(m_outputImage, outputs);

    Q_EMIT dataUpdated(0);
}

cv::Mat YOLOObjectDetectorModel::preprocessImage(const cv::Mat& image)
{
    // Get input size
    int sizeIndex = m_inputSizeCombo->currentData().toInt();
    cv::Size inputSize(320, 320);
    switch (sizeIndex)
    {
        case 0: inputSize = cv::Size(320, 320); break;
        case 1: inputSize = cv::Size(416, 416); break;
        case 2: inputSize = cv::Size(512, 512); break;
        case 3: inputSize = cv::Size(608, 608); break;
        case 4: inputSize = cv::Size(640, 640); break;
    }

    // Create blob from image
    cv::Mat blob = cv::dnn::blobFromImage(
        image,
        m_inputScale,
        inputSize,
        m_mean,
        m_swapRB,
        false,
        CV_32F
    );

    return blob;
}

void YOLOObjectDetectorModel::postprocessAndDraw(cv::Mat& image,
                                                  const std::vector<cv::Mat>& outputs)
{
    m_detections.clear();

    // Get output dimensions based on YOLO version
    int version = m_yoloVersionCombo->currentData().toInt();

    if (version == 2)
    {
        // YOLOv5/ONNX format: [1, 25200, 85] for COCO
        // Output format: [batch, num_detections, num_classes + 5]
        // 5 = [x, y, w, h, confidence]

        cv::Mat output = outputs[0];
        int numDetections = output.size[1];
        int numClasses = output.size[2] - 5;

        // Reshape if needed
        if (output.dims == 3)
        {
            output = output.reshape(1, output.size[1]);
        }

        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        // Process each detection
        for (int i = 0; i < numDetections; ++i)
        {
            // Get confidence scores
            cv::Mat scores = output.row(i).colRange(5, output.cols);
            cv::Point classIdPoint;
            double maxScore;
            cv::minMaxLoc(scores, 0, &maxScore, 0, &classIdPoint);

            if (maxScore >= m_confidenceThreshold)
            {
                // Get box coordinates (center x, center y, width, height)
                float centerX = output.at<float>(i, 0);
                float centerY = output.at<float>(i, 1);
                float width = output.at<float>(i, 2);
                float height = output.at<float>(i, 3);

                // Scale to image size
                int left = static_cast<int>((centerX - width / 2) * image.cols);
                int top = static_cast<int>((centerY - height / 2) * image.rows);
                int right = static_cast<int>((centerX + width / 2) * image.cols);
                int bottom = static_cast<int>((centerY + height / 2) * image.rows);

                // Clip to image bounds
                left = std::max(0, std::min(left, image.cols - 1));
                top = std::max(0, std::min(top, image.rows - 1));
                right = std::max(0, std::min(right, image.cols - 1));
                bottom = std::max(0, std::min(bottom, image.rows - 1));

                classIds.push_back(classIdPoint.x);
                confidences.push_back(static_cast<float>(maxScore));
                boxes.push_back(cv::Rect(left, top, right - left, bottom - top));
            }
        }

        // Apply non-maximum suppression
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, m_confidenceThreshold,
                         m_nmsThreshold, indices);

        // Draw detections
        for (size_t i = 0; i < indices.size(); ++i)
        {
            int idx = indices[i];
            cv::Rect box = boxes[idx];
            int classId = classIds[idx];
            float confidence = confidences[idx];

            Detection det;
            det.classId = classId;
            det.confidence = confidence;
            det.box = box;
            m_detections.push_back(det);

            if (m_showBoxes)
            {
                // Generate color based on class ID
                cv::Scalar color =
                    cv::Scalar((classId * 37) % 256,
                              (classId * 97) % 256,
                              (classId * 151) % 256);
                cv::rectangle(image, box, color, 2);

                if (m_showLabels)
                {
                    std::string label;
                    if (classId < static_cast<int>(m_classNames.size()))
                    {
                        label = m_classNames[classId];
                    }
                    else
                    {
                        label = "Class_" + std::to_string(classId);
                    }

                    if (m_showConfidence)
                    {
                        label += ": " + std::to_string(static_cast<int>(confidence * 100)) + "%";
                    }

                    int baseLine;
                    cv::Size labelSize =
                        cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                       0.5, 1, &baseLine);
                    cv::rectangle(image,
                                cv::Point(box.x, box.y - round(labelSize.height)),
                                cv::Point(box.x + round(labelSize.width),
                                         box.y + baseLine),
                                color, cv::FILLED);
                    cv::putText(image, label,
                               cv::Point(box.x, box.y),
                               cv::FONT_HERSHEY_SIMPLEX, 0.5,
                               cv::Scalar(255, 255, 255), 1);
                }
            }
        }
    }
    else
    {
        // YOLOv3/v4 format
        // Multiple output layers for different scales
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        for (const cv::Mat& output : outputs)
        {
            int numDetections = output.rows;
            int numClasses = output.cols - 5;

            for (int i = 0; i < numDetections; ++i)
            {
                const float* data = output.ptr<float>(i);

                // Get confidence
                float confidence = data[4];
                if (confidence >= m_confidenceThreshold)
                {
                    cv::Mat scores(1, numClasses, CV_32F, const_cast<float*>(data + 5));
                    cv::Point classIdPoint;
                    double maxScore;
                    cv::minMaxLoc(scores, 0, &maxScore, 0, &classIdPoint);

                    if (maxScore >= m_confidenceThreshold)
                    {
                        // Get box coordinates (center x, center y, width, height)
                        float centerX = data[0] * image.cols;
                        float centerY = data[1] * image.rows;
                        float width = data[2] * image.cols;
                        float height = data[3] * image.rows;

                        int left = static_cast<int>(centerX - width / 2);
                        int top = static_cast<int>(centerY - height / 2);

                        classIds.push_back(classIdPoint.x);
                        confidences.push_back(static_cast<float>(maxScore));
                        boxes.push_back(cv::Rect(left, top,
                                               static_cast<int>(width),
                                               static_cast<int>(height)));
                    }
                }
            }
        }

        // Apply NMS
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, m_confidenceThreshold,
                         m_nmsThreshold, indices);

        // Draw detections
        for (size_t i = 0; i < indices.size(); ++i)
        {
            int idx = indices[i];
            cv::Rect box = boxes[idx];
            int classId = classIds[idx];
            float confidence = confidences[idx];

            Detection det;
            det.classId = classId;
            det.confidence = confidence;
            det.box = box;
            m_detections.push_back(det);

            if (m_showBoxes)
            {
                cv::Scalar color =
                    cv::Scalar((classId * 37) % 256,
                              (classId * 97) % 256,
                              (classId * 151) % 256);
                cv::rectangle(image, box, color, 2);

                if (m_showLabels)
                {
                    std::string label;
                    if (classId < static_cast<int>(m_classNames.size()))
                    {
                        label = m_classNames[classId];
                    }
                    else
                    {
                        label = "Class_" + std::to_string(classId);
                    }

                    if (m_showConfidence)
                    {
                        label += ": " + std::to_string(static_cast<int>(confidence * 100)) + "%";
                    }

                    int baseLine;
                    cv::Size labelSize =
                        cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                       0.5, 1, &baseLine);
                    cv::rectangle(image,
                                cv::Point(box.x, box.y - round(labelSize.height)),
                                cv::Point(box.x + round(labelSize.width),
                                         box.y + baseLine),
                                color, cv::FILLED);
                    cv::putText(image, label,
                               cv::Point(box.x, box.y),
                               cv::FONT_HERSHEY_SIMPLEX, 0.5,
                               cv::Scalar(255, 255, 255), 1);
                }
            }
        }
    }

    // Update info text
    QString info = QString("Detected %1 objects").arg(m_detections.size());
    m_infoText->setText(info);
}

std::vector<int> YOLOObjectDetectorModel::getOutputLayers(const cv::dnn::Net& net)
{
    std::vector<int> outLayers = net.getUnconnectedOutLayers();
    return outLayers;
}

cv::Mat YOLOObjectDetectorModel::getOutputBlob(const std::vector<cv::Mat>& outputs)
{
    // Combine multiple output layers into single blob
    // Implementation depends on specific YOLO version
    return outputs[0].clone();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject YOLOObjectDetectorModel::save() const
{
    QJsonObject modelJson;
    modelJson["modelPath"] = m_modelPath;
    modelJson["configPath"] = m_configPath;
    modelJson["classesPath"] = m_classesPath;
    modelJson["yoloVersion"] = m_yoloVersion;
    modelJson["confidenceThreshold"] = m_confidenceThreshold;
    modelJson["nmsThreshold"] = m_nmsThreshold;
    modelJson["inputSizeIndex"] = m_inputSizeIndex;
    modelJson["backendIndex"] = m_backendIndex;
    modelJson["showBoxes"] = m_showBoxes;
    modelJson["showLabels"] = m_showLabels;
    return modelJson;
}

void YOLOObjectDetectorModel::load(QJsonObject const& model)
{
    QJsonValue modelJson = model["modelPath"];
    if (!modelJson.isUndefined())
    {
        m_modelPath = modelJson.toString();
        m_modelEdit->setText(m_modelPath);
    }

    QJsonValue configJson = model["configPath"];
    if (!configJson.isUndefined())
    {
        m_configPath = configJson.toString();
        m_configEdit->setText(m_configPath);
    }

    QJsonValue classesJson = model["classesPath"];
    if (!classesJson.isUndefined())
    {
        m_classesPath = classesJson.toString();
        m_classesEdit->setText(m_classesPath);
        loadClasses();
    }

    QJsonValue versionJson = model["yoloVersion"];
    if (!versionJson.isUndefined())
    {
        m_yoloVersion = versionJson.toInt();
        for (int i = 0; i < m_yoloVersionCombo->count(); ++i)
        {
            if (m_yoloVersionCombo->itemData(i).toInt() == m_yoloVersion)
            {
                m_yoloVersionCombo->blockSignals(true);
                m_yoloVersionCombo->setCurrentIndex(i);
                m_yoloVersionCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue confJson = model["confidenceThreshold"];
    if (!confJson.isUndefined())
    {
        m_confidenceThreshold = confJson.toDouble();
        m_confidenceSpin->setValue(m_confidenceThreshold);
    }

    QJsonValue nmsJson = model["nmsThreshold"];
    if (!nmsJson.isUndefined())
    {
        m_nmsThreshold = nmsJson.toDouble();
        m_nmsSpin->setValue(m_nmsThreshold);
    }

    QJsonValue sizeJson = model["inputSizeIndex"];
    if (!sizeJson.isUndefined())
    {
        m_inputSizeIndex = sizeJson.toInt();
        m_inputSizeCombo->blockSignals(true);
        m_inputSizeCombo->setCurrentIndex(m_inputSizeIndex);
        m_inputSizeCombo->blockSignals(false);
    }

    QJsonValue backendJson = model["backendIndex"];
    if (!backendJson.isUndefined())
    {
        m_backendIndex = backendJson.toInt();
        m_backendCombo->blockSignals(true);
        m_backendCombo->setCurrentIndex(m_backendIndex);
        m_backendCombo->blockSignals(false);
    }

    QJsonValue boxesJson = model["showBoxes"];
    if (!boxesJson.isUndefined())
    {
        m_showBoxes = boxesJson.toBool();
        m_showBoxesCheck->setChecked(m_showBoxes);
    }

    QJsonValue labelsJson = model["showLabels"];
    if (!labelsJson.isUndefined())
    {
        m_showLabels = labelsJson.toBool();
        m_showLabelsCheck->setChecked(m_showLabels);
    }

    // Auto-load model if paths are available
    if (!m_modelPath.isEmpty())
    {
        bool hasConfig = true;
        if (m_yoloVersion < 2)
        {
            hasConfig = !m_configPath.isEmpty();
        }

        if (hasConfig)
        {
            loadModel();
        }
    }
}

} // namespace VisionBox
