/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * DNN Inference Model Implementation
 ******************************************************************************/

#include "DNNInferenceModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <fstream>
#include <sstream>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
DNNInferenceModel::DNNInferenceModel()
    : m_outputImage(nullptr)
    , m_net(nullptr)
{
    // Initialize empty network
    // m_net = cv::dnn::readNetFromONNX("");

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Model file
    auto* modelLayout = new QHBoxLayout();
    modelLayout->addWidget(new QLabel("Model:"));
    m_modelPathEdit = new QLineEdit();
    m_modelPathEdit->setReadOnly(true);
    m_modelPathEdit->setPlaceholderText("ONNX model file (.onnx)");
    modelLayout->addWidget(m_modelPathEdit);
    m_modelBrowseBtn = new QPushButton("Browse...");
    modelLayout->addWidget(m_modelBrowseBtn);
    layout->addLayout(modelLayout);

    // Config file (for Caffe/TensorFlow)
    auto* configLayout = new QHBoxLayout();
    configLayout->addWidget(new QLabel("Config:"));
    m_configPathEdit = new QLineEdit();
    m_configPathEdit->setReadOnly(true);
    m_configPathEdit->setPlaceholderText("Optional config file (.prototxt)");
    configLayout->addWidget(m_configPathEdit);
    m_configBrowseBtn = new QPushButton("Browse...");
    configLayout->addWidget(m_configBrowseBtn);
    layout->addLayout(configLayout);

    // Classes file
    auto* classesLayout = new QHBoxLayout();
    classesLayout->addWidget(new QLabel("Classes:"));
    m_classesPathEdit = new QLineEdit();
    m_classesPathEdit->setReadOnly(true);
    m_classesPathEdit->setPlaceholderText("Optional class names file");
    classesLayout->addWidget(m_classesPathEdit);
    m_classesBrowseBtn = new QPushButton("Browse...");
    classesLayout->addWidget(m_classesBrowseBtn);
    layout->addLayout(classesLayout);

    // Load button
    m_loadBtn = new QPushButton("Load Model");
    m_loadBtn->setEnabled(false);
    layout->addWidget(m_loadBtn);

    // Backend
    auto* backendLayout = new QHBoxLayout();
    backendLayout->addWidget(new QLabel("Backend:"));
    m_backendCombo = new QComboBox();
    m_backendCombo->addItem("OpenCV", cv::dnn::DNN_BACKEND_OPENCV);
    m_backendCombo->addItem("CUDA", cv::dnn::DNN_BACKEND_CUDA);
    m_backendCombo->addItem("Inference Engine", cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
    m_backendCombo->addItem("Variance", cv::dnn::DNN_BACKEND_VKCOM);
    m_backendCombo->addItem("CPU", cv::dnn::DNN_BACKEND_DEFAULT);
    m_backendCombo->setCurrentIndex(0);
    backendLayout->addWidget(m_backendCombo);
    layout->addLayout(backendLayout);

    // Target
    auto* targetLayout = new QHBoxLayout();
    targetLayout->addWidget(new QLabel("Target:"));
    m_targetCombo = new QComboBox();
    m_targetCombo->addItem("CPU", cv::dnn::DNN_TARGET_CPU);
    m_targetCombo->addItem("CUDA FP32", cv::dnn::DNN_TARGET_CUDA);
    m_targetCombo->addItem("CUDA FP16", cv::dnn::DNN_TARGET_CUDA_FP16);
    m_targetCombo->addItem("FPGA", cv::dnn::DNN_TARGET_FPGA);
    m_targetCombo->addItem("Vulkan", cv::dnn::DNN_TARGET_VULKAN);
    m_targetCombo->addItem("OpenCL", cv::dnn::DNN_TARGET_OPENCL);
    m_targetCombo->addItem("OpenCL FP16", cv::dnn::DNN_TARGET_OPENCL_FP16);
    m_targetCombo->setCurrentIndex(0);
    targetLayout->addWidget(m_targetCombo);
    layout->addLayout(targetLayout);

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
    nmsLayout->addWidget(new QLabel("NMS:"));
    m_nmsSpin = new QDoubleSpinBox();
    m_nmsSpin->setRange(0.0, 1.0);
    m_nmsSpin->setSingleStep(0.05);
    m_nmsSpin->setValue(0.4);
    nmsLayout->addWidget(m_nmsSpin);
    layout->addLayout(nmsLayout);

    // Swap RB
    m_swapRBCheck = new QCheckBox("Swap RB (BGR to RGB)");
    m_swapRBCheck->setChecked(true);
    layout->addWidget(m_swapRBCheck);

    // Status label
    m_statusLabel = new QLabel("Status: No model loaded");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");
    layout->addWidget(m_statusLabel);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_modelBrowseBtn, &QPushButton::clicked,
            this, &DNNInferenceModel::onBrowseModel);
    connect(m_configBrowseBtn, &QPushButton::clicked,
            this, &DNNInferenceModel::onBrowseConfig);
    connect(m_classesBrowseBtn, &QPushButton::clicked,
            this, &DNNInferenceModel::onBrowseClasses);
    connect(m_loadBtn, &QPushButton::clicked,
            this, &DNNInferenceModel::onLoadModel);
    connect(m_backendCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DNNInferenceModel::onBackendChanged);
    connect(m_targetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DNNInferenceModel::onTargetChanged);
    connect(m_confidenceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DNNInferenceModel::onConfidenceChanged);
    connect(m_nmsSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this]() { m_nmsThreshold = m_nmsSpin->value(); runInference(); });
    connect(m_swapRBCheck, &QCheckBox::stateChanged,
            this, &DNNInferenceModel::onSwapRBChanged);
}

DNNInferenceModel::~DNNInferenceModel()
{
    m_net = nullptr;
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int DNNInferenceModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType DNNInferenceModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> DNNInferenceModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void DNNInferenceModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                  QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    runInference();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* DNNInferenceModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void DNNInferenceModel::onBrowseModel()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open DNN Model",
        "",
        "ONNX Models (*.onnx);;TensorFlow Models (*.pb);;Caffe Models (*.caffemodel);;All Files (*.*)"
    );

    if (!filePath.isEmpty())
    {
        m_modelPath = filePath;
        m_modelPathEdit->setText(filePath);
        m_loadBtn->setEnabled(true);
    }
}

void DNNInferenceModel::onBrowseConfig()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open Config File",
        "",
        "Prototxt (*.prototxt);;PBtxt (*.pbtxt);;All Files (*.*)"
    );

    if (!filePath.isEmpty())
    {
        m_configPath = filePath;
        m_configPathEdit->setText(filePath);
    }
}

void DNNInferenceModel::onBrowseClasses()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open Classes File",
        "",
        "Text Files (*.txt);;All Files (*.*)"
    );

    if (!filePath.isEmpty())
    {
        m_classesPath = filePath;
        m_classesPathEdit->setText(filePath);
        loadModelFiles();
    }
}

void DNNInferenceModel::onLoadModel()
{
    loadModelFiles();
}

void DNNInferenceModel::onBackendChanged()
{
    m_backend = m_backendCombo->currentData().toInt();

    if (m_modelLoaded)
    {
        m_net->setPreferableBackend(m_backend);
        runInference();
    }
}

void DNNInferenceModel::onTargetChanged()
{
    m_target = m_targetCombo->currentData().toInt();

    if (m_modelLoaded)
    {
        m_net->setPreferableTarget(m_target);
        runInference();
    }
}

void DNNInferenceModel::onConfidenceChanged(double value)
{
    m_confidenceThreshold = value;
    runInference();
}

void DNNInferenceModel::onSwapRBChanged(int state)
{
    m_swapRB = (state == Qt::Checked);
    runInference();
}

/*******************************************************************************
 * DNN Inference
 ******************************************************************************/
void DNNInferenceModel::loadModelFiles()
{
    try
    {
        // Load model based on file extension
        cv::dnn::Net net;
        if (m_modelPath.endsWith(".onnx", Qt::CaseInsensitive))
        {
            net = cv::dnn::readNetFromONNX(m_modelPath.toStdString());
        }
        else if (m_modelPath.endsWith(".pb", Qt::CaseInsensitive))
        {
            if (!m_configPath.isEmpty())
            {
                net = cv::dnn::readNetFromTensorflow(m_configPath.toStdString(),
                                                       m_modelPath.toStdString());
            }
            else
            {
                net = cv::dnn::readNetFromTensorflow(m_modelPath.toStdString());
            }
        }
        else if (m_modelPath.endsWith(".caffemodel", Qt::CaseInsensitive))
        {
            if (!m_configPath.isEmpty())
            {
                net = cv::dnn::readNetFromCaffe(m_configPath.toStdString(),
                                                  m_modelPath.toStdString());
            }
            else
            {
                m_statusLabel->setText("Status: Caffe models require config file");
                m_modelLoaded = false;
                return;
            }
        }
        else
        {
            m_statusLabel->setText("Status: Unsupported model format");
            m_modelLoaded = false;
            return;
        }

        m_net = cv::makePtr<cv::dnn::Net>(net);

        if (m_net->empty())
        {
            m_statusLabel->setText("Status: Failed to load model");
            m_modelLoaded = false;
            return;
        }

        // Set backend and target
        m_net->setPreferableBackend(m_backend);
        m_net->setPreferableTarget(m_target);

        // Load class names if provided
        if (!m_classesPath.isEmpty())
        {
            std::ifstream ifs(m_classesPath.toStdString());
            std::string line;
            m_classes.clear();
            while (std::getline(ifs, line))
            {
                if (!line.empty())
                {
                    m_classes.push_back(line);
                }
            }
        }

        m_modelLoaded = true;
        m_statusLabel->setText(QString("Status: Model loaded (%1 classes)")
                               .arg(m_classes.size()));

        runInference();
    }
    catch (const cv::Exception& e)
    {
        m_statusLabel->setText(QString("Status: Error - %1").arg(e.what()));
        m_modelLoaded = false;
    }
}

void DNNInferenceModel::preprocessImage(const cv::Mat& image, cv::Mat& blob,
                                        cv::Size& inputSize)
{
    // Default input size
    inputSize = cv::Size(640, 640);

    // Create blob from image
    blob = cv::dnn::blobFromImage(image, m_scale, inputSize, m_mean,
                                   m_swapRB, false, CV_32F);
}

void DNNInferenceModel::runInference()
{
    if (!m_inputImage || !m_modelLoaded)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
        return;
    }

    cv::Mat input = m_inputImage->image();

    if (input.empty())
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
        return;
    }

    try
    {
        // Preprocess image
        cv::Mat blob;
        cv::Size inputSize;
        preprocessImage(input, blob, inputSize);

        // Set input to the network
        m_net->setInput(blob);

        // Forward pass
        std::vector<cv::Mat> outs;
        m_net->forward(outs);

        // Visualize detections
        cv::Mat output = visualizeDetections(input, outs);

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

cv::Mat DNNInferenceModel::visualizeDetections(const cv::Mat& image,
                                              const std::vector<cv::Mat>& outs)
{
    cv::Mat result = image.clone();

    // For object detection models (like YOLO), outputs typically have shape:
    // [1, num_detections, 85] where 85 = (x, y, w, h, confidence, class_scores...)

    // This is a simplified visualization - real implementation depends on model type
    // For now, just return the original image with a note
    cv::putText(result, "DNN Inference Complete", cv::Point(10, 30),
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

    if (!m_classes.empty())
    {
        cv::putText(result, QString("Classes: %1").arg(m_classes.size()).toStdString(),
                   cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                   cv::Scalar(0, 255, 0), 2);
    }

    return result;
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject DNNInferenceModel::save() const
{
    QJsonObject modelJson;
    modelJson["modelPath"] = m_modelPath;
    modelJson["configPath"] = m_configPath;
    modelJson["classesPath"] = m_classesPath;
    modelJson["backend"] = m_backend;
    modelJson["target"] = m_target;
    modelJson["confidenceThreshold"] = m_confidenceThreshold;
    modelJson["nmsThreshold"] = m_nmsThreshold;
    modelJson["swapRB"] = m_swapRB;
    return modelJson;
}

void DNNInferenceModel::load(QJsonObject const& model)
{
    QJsonValue modelPathJson = model["modelPath"];
    if (!modelPathJson.isUndefined() && !modelPathJson.toString().isEmpty())
    {
        m_modelPath = modelPathJson.toString();
        m_modelPathEdit->setText(m_modelPath);
        m_loadBtn->setEnabled(true);
    }

    QJsonValue configPathJson = model["configPath"];
    if (!configPathJson.isUndefined())
    {
        m_configPath = configPathJson.toString();
        m_configPathEdit->setText(m_configPath);
    }

    QJsonValue classesPathJson = model["classesPath"];
    if (!classesPathJson.isUndefined())
    {
        m_classesPath = classesPathJson.toString();
        m_classesPathEdit->setText(m_classesPath);
    }

    QJsonValue backendJson = model["backend"];
    if (!backendJson.isUndefined())
    {
        m_backend = backendJson.toInt();
        for (int i = 0; i < m_backendCombo->count(); ++i)
        {
            if (m_backendCombo->itemData(i).toInt() == m_backend)
            {
                m_backendCombo->blockSignals(true);
                m_backendCombo->setCurrentIndex(i);
                m_backendCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue targetJson = model["target"];
    if (!targetJson.isUndefined())
    {
        m_target = targetJson.toInt();
        for (int i = 0; i < m_targetCombo->count(); ++i)
        {
            if (m_targetCombo->itemData(i).toInt() == m_target)
            {
                m_targetCombo->blockSignals(true);
                m_targetCombo->setCurrentIndex(i);
                m_targetCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue confidenceJson = model["confidenceThreshold"];
    if (!confidenceJson.isUndefined())
    {
        m_confidenceThreshold = confidenceJson.toDouble();
        m_confidenceSpin->setValue(m_confidenceThreshold);
    }

    QJsonValue nmsJson = model["nmsThreshold"];
    if (!nmsJson.isUndefined())
    {
        m_nmsThreshold = nmsJson.toDouble();
        m_nmsSpin->setValue(m_nmsThreshold);
    }

    QJsonValue swapRBJson = model["swapRB"];
    if (!swapRBJson.isUndefined())
    {
        m_swapRB = swapRBJson.toBool();
        m_swapRBCheck->setChecked(m_swapRB);
    }

    // Auto-load model if path is set
    if (!m_modelPath.isEmpty())
    {
        loadModelFiles();
    }
}

} // namespace VisionBox
