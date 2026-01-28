/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Haar Face Detection Model Implementation
 ******************************************************************************/

#include "HaarFaceDetectionModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
HaarFaceDetectionModel::HaarFaceDetectionModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Detection type selector
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Detect:"));
    m_detectionTypeCombo = new QComboBox();
    m_detectionTypeCombo->addItem("Faces", static_cast<int>(DetectionType::Face));
    m_detectionTypeCombo->addItem("Eyes", static_cast<int>(DetectionType::Eyes));
    m_detectionTypeCombo->addItem("Smiles", static_cast<int>(DetectionType::Smile));
    typeLayout->addWidget(m_detectionTypeCombo);
    layout->addLayout(typeLayout);

    // Scale factor
    auto* scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("Scale Factor:"));
    m_scaleFactorSpin = new QDoubleSpinBox();
    m_scaleFactorSpin->setRange(1.01, 2.0);
    m_scaleFactorSpin->setSingleStep(0.05);
    m_scaleFactorSpin->setDecimals(2);
    m_scaleFactorSpin->setValue(1.1);
    scaleLayout->addWidget(m_scaleFactorSpin);
    layout->addLayout(scaleLayout);

    // Min neighbors
    auto* neighborsLayout = new QHBoxLayout();
    neighborsLayout->addWidget(new QLabel("Min Neighbors:"));
    m_minNeighborsSpin = new QSpinBox();
    m_minNeighborsSpin->setRange(1, 10);
    m_minNeighborsSpin->setValue(3);
    neighborsLayout->addWidget(m_minNeighborsSpin);
    layout->addLayout(neighborsLayout);

    // Min size
    auto* minSizeLayout = new QHBoxLayout();
    minSizeLayout->addWidget(new QLabel("Min Size:"));
    m_minSizeSpin = new QSpinBox();
    m_minSizeSpin->setRange(10, 200);
    m_minSizeSpin->setValue(30);
    minSizeLayout->addWidget(m_minSizeSpin);
    layout->addLayout(minSizeLayout);

    // Draw bounding boxes
    m_drawBoxesCheck = new QCheckBox("Draw Bounding Boxes");
    m_drawBoxesCheck->setChecked(true);
    layout->addWidget(m_drawBoxesCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    // Load default Haar cascade files
    loadCascade("/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");
    loadCascade("/usr/share/opencv4/haarcascades/haarcascade_eye.xml");
    loadCascade("/usr/share/opencv4/haarcascades/haarcascade_smile.xml");

    connect(m_detectionTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HaarFaceDetectionModel::onDetectionTypeChanged);
    connect(m_scaleFactorSpin, &QDoubleSpinBox::valueChanged,
            this, &HaarFaceDetectionModel::onScaleFactorChanged);
    connect(m_minNeighborsSpin, &QSpinBox::valueChanged,
            this, &HaarFaceDetectionModel::onMinNeighborsChanged);
    connect(m_minSizeSpin, &QSpinBox::valueChanged,
            this, &HaarFaceDetectionModel::onMinSizeChanged);
    connect(m_drawBoxesCheck, &QCheckBox::stateChanged,
            this, &HaarFaceDetectionModel::onDrawBoxesChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int HaarFaceDetectionModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1;
    }
    else
    {
        return 1;
    }
}

QtNodes::NodeDataType HaarFaceDetectionModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> HaarFaceDetectionModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void HaarFaceDetectionModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                       QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    detectFaces();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* HaarFaceDetectionModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void HaarFaceDetectionModel::onDetectionTypeChanged(int index)
{
    m_detectionType = static_cast<DetectionType>(m_detectionTypeCombo->itemData(index).toInt());
    detectFaces();
}

void HaarFaceDetectionModel::onScaleFactorChanged(double value)
{
    m_scaleFactor = value;
    detectFaces();
}

void HaarFaceDetectionModel::onMinNeighborsChanged(int value)
{
    m_minNeighbors = value;
    detectFaces();
}

void HaarFaceDetectionModel::onMinSizeChanged(int value)
{
    m_minSize = value;
    detectFaces();
}

void HaarFaceDetectionModel::onDrawBoxesChanged(int state)
{
    m_drawBoxes = (state == Qt::Checked);
    detectFaces();
}

/*******************************************************************************
 * Cascade Loading
 ******************************************************************************/
bool HaarFaceDetectionModel::loadCascade(const std::string& cascadePath)
{
    // Try to load cascade from common locations
    std::vector<std::string> possiblePaths = {
        cascadePath,
        "/usr/local/share/opencv4/haarcascades/" +
            cascadePath.substr(cascadePath.find_last_of('/') + 1),
        "../haarcascades/" +
            cascadePath.substr(cascadePath.find_last_of('/') + 1)
    };

    for (const auto& path : possiblePaths)
    {
        if (path.find("frontalface") != std::string::npos)
        {
            if (m_faceCascade.load(path))
                return true;
        }
        else if (path.find("eye") != std::string::npos)
        {
            if (m_eyeCascade.load(path))
                return true;
        }
        else if (path.find("smile") != std::string::npos)
        {
            if (m_smileCascade.load(path))
                return true;
        }
    }

    return false;
}

/*******************************************************************************
 * Face Detection
 ******************************************************************************/
void HaarFaceDetectionModel::detectFaces()
{
    if (!m_inputImage)
    {
        m_outputImage = nullptr;
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
        // Convert to grayscale if needed
        cv::Mat gray;
        if (input.channels() > 1)
        {
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = input;
        }

        // Equalize histogram for better detection
        cv::equalizeHist(gray, gray);

        // Create output image
        cv::Mat output;
        if (input.channels() == 1)
        {
            cv::cvtColor(input, output, cv::COLOR_GRAY2BGR);
        }
        else
        {
            output = input.clone();
        }

        // Select cascade based on detection type
        cv::CascadeClassifier* cascade = nullptr;
        cv::Scalar boxColor;
        std::string labelPrefix;

        switch (m_detectionType)
        {
            case DetectionType::Face:
                cascade = &m_faceCascade;
                boxColor = cv::Scalar(255, 0, 0);  // Blue in BGR
                labelPrefix = "Face";
                break;
            case DetectionType::Eyes:
                cascade = &m_eyeCascade;
                boxColor = cv::Scalar(0, 255, 0);  // Green in BGR
                labelPrefix = "Eye";
                break;
            case DetectionType::Smile:
                cascade = &m_smileCascade;
                boxColor = cv::Scalar(0, 0, 255);  // Red in BGR
                labelPrefix = "Smile";
                break;
        }

        if (!cascade || cascade->empty())
        {
            // Cascade not loaded
            m_outputImage = std::make_shared<ImageData>(output);
            Q_EMIT dataUpdated(0);
            return;
        }

        // Detect objects
        std::vector<cv::Rect> objects;
        cv::Size minSize(m_minSize, m_minSize);

        cascade->detectMultiScale(gray, objects, m_scaleFactor, m_minNeighbors,
                                 0, minSize);

        // Draw bounding boxes
        if (m_drawBoxes)
        {
            for (size_t i = 0; i < objects.size(); ++i)
            {
                const cv::Rect& rect = objects[i];

                // Draw rectangle
                cv::rectangle(output, rect, boxColor, 2);

                // Add label
                std::string label = labelPrefix + " " + std::to_string(i + 1);
                int baseline = 0;
                cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                                   0.5, 1, &baseline);

                // Draw label background
                cv::Rect labelRect(rect.x, rect.y - textSize.height - 5,
                                 textSize.width + 10, textSize.height + 10);
                cv::rectangle(output, labelRect, boxColor, -1);

                // Draw label text
                cv::putText(output, label,
                          cv::Point(rect.x + 5, rect.y - 5),
                          cv::FONT_HERSHEY_SIMPLEX, 0.5,
                          cv::Scalar(255, 255, 255), 1);
            }
        }

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject HaarFaceDetectionModel::save() const
{
    QJsonObject modelJson;
    modelJson["detectionType"] = static_cast<int>(m_detectionType);
    modelJson["scaleFactor"] = m_scaleFactor;
    modelJson["minNeighbors"] = m_minNeighbors;
    modelJson["minSize"] = m_minSize;
    modelJson["drawBoxes"] = m_drawBoxes;
    return modelJson;
}

void HaarFaceDetectionModel::load(QJsonObject const& model)
{
    QJsonValue typeJson = model["detectionType"];
    if (!typeJson.isUndefined())
    {
        m_detectionType = static_cast<DetectionType>(typeJson.toInt());
        m_detectionTypeCombo->setCurrentIndex(static_cast<int>(m_detectionType));
    }

    QJsonValue scaleJson = model["scaleFactor"];
    if (!scaleJson.isUndefined())
    {
        m_scaleFactor = scaleJson.toDouble();
        m_scaleFactorSpin->setValue(m_scaleFactor);
    }

    QJsonValue neighborsJson = model["minNeighbors"];
    if (!neighborsJson.isUndefined())
    {
        m_minNeighbors = neighborsJson.toInt();
        m_minNeighborsSpin->setValue(m_minNeighbors);
    }

    QJsonValue minSizeJson = model["minSize"];
    if (!minSizeJson.isUndefined())
    {
        m_minSize = minSizeJson.toInt();
        m_minSizeSpin->setValue(m_minSize);
    }

    QJsonValue drawJson = model["drawBoxes"];
    if (!drawJson.isUndefined())
    {
        m_drawBoxes = drawJson.toBool();
        m_drawBoxesCheck->setChecked(m_drawBoxes);
    }

    detectFaces();
}

} // namespace VisionBox
