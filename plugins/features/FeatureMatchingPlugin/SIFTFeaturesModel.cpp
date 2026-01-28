/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * SIFT Features Model Implementation
 ******************************************************************************/

#include "SIFTFeaturesModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
SIFTFeaturesModel::SIFTFeaturesModel()
    : m_outputImage(nullptr)
    , m_keypointData(nullptr)
{
    // Create SIFT detector
    m_sift = cv::SIFT::create(
        m_nFeatures,
        m_nOctaveLayers,
        m_contrastThreshold,
        m_edgeThreshold,
        m_sigma
    );

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Number of features
    auto* featuresLayout = new QHBoxLayout();
    featuresLayout->addWidget(new QLabel("Max Features:"));
    m_featuresSpin = new QSpinBox();
    m_featuresSpin->setRange(0, 10000);
    m_featuresSpin->setValue(0);
    m_featuresSpin->setToolTip("0 = detect all features");
    featuresLayout->addWidget(m_featuresSpin);
    layout->addLayout(featuresLayout);

    // Octave layers
    auto* octaveLayout = new QHBoxLayout();
    octaveLayout->addWidget(new QLabel("Octave Layers:"));
    m_octaveLayersSpin = new QSpinBox();
    m_octaveLayersSpin->setRange(1, 10);
    m_octaveLayersSpin->setValue(3);
    octaveLayout->addWidget(m_octaveLayersSpin);
    layout->addLayout(octaveLayout);

    // Contrast threshold
    auto* contrastLayout = new QHBoxLayout();
    contrastLayout->addWidget(new QLabel("Contrast Thresh:"));
    m_contrastThresholdSpin = new QDoubleSpinBox();
    m_contrastThresholdSpin->setRange(0.0, 1.0);
    m_contrastThresholdSpin->setSingleStep(0.01);
    m_contrastThresholdSpin->setValue(0.04);
    m_contrastThresholdSpin->setToolTip("Higher = fewer features");
    contrastLayout->addWidget(m_contrastThresholdSpin);
    layout->addLayout(contrastLayout);

    // Edge threshold
    auto* edgeLayout = new QHBoxLayout();
    edgeLayout->addWidget(new QLabel("Edge Thresh:"));
    m_edgeThresholdSpin = new QDoubleSpinBox();
    m_edgeThresholdSpin->setRange(1.0, 50.0);
    m_edgeThresholdSpin->setSingleStep(1.0);
    m_edgeThresholdSpin->setValue(10.0);
    m_edgeThresholdSpin->setToolTip("Higher = more features");
    edgeLayout->addWidget(m_edgeThresholdSpin);
    layout->addLayout(edgeLayout);

    // Sigma
    auto* sigmaLayout = new QHBoxLayout();
    sigmaLayout->addWidget(new QLabel("Sigma:"));
    m_sigmaSpin = new QDoubleSpinBox();
    m_sigmaSpin->setRange(0.1, 10.0);
    m_sigmaSpin->setSingleStep(0.1);
    m_sigmaSpin->setValue(1.6);
    sigmaLayout->addWidget(m_sigmaSpin);
    layout->addLayout(sigmaLayout);

    // Show keypoints
    m_showKeypointsCheck = new QCheckBox("Show Keypoints");
    m_showKeypointsCheck->setChecked(true);
    layout->addWidget(m_showKeypointsCheck);

    // Keypoints count label
    m_keypointsLabel = new QLabel("Keypoints: 0");
    m_keypointsLabel->setStyleSheet("QLabel { font-weight: bold; padding: 5px; }");
    layout->addWidget(m_keypointsLabel);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_featuresSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SIFTFeaturesModel::onFeaturesChanged);
    connect(m_octaveLayersSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SIFTFeaturesModel::onOctaveLayersChanged);
    connect(m_contrastThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SIFTFeaturesModel::onContrastThresholdChanged);
    connect(m_edgeThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SIFTFeaturesModel::onEdgeThresholdChanged);
    connect(m_sigmaSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &SIFTFeaturesModel::onSigmaChanged);
    connect(m_showKeypointsCheck, &QCheckBox::stateChanged,
            this, &SIFTFeaturesModel::onShowKeypointsChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int SIFTFeaturesModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image
    }
    else
    {
        return 2; // Output: visualization and keypoints
    }
}

QtNodes::NodeDataType SIFTFeaturesModel::dataType(
    QtNodes::PortType portType,
    QtNodes::PortIndex portIndex) const
{
    if (portType == QtNodes::PortType::Out)
    {
        if (portIndex == 0)
        {
            return ImageData().type(); // Visualization
        }
        else
        {
            return KeypointData().type(); // Keypoints
        }
    }
    return ImageData().type();
}

/*******************************************************************************
 * Data Flow
 ******************************************************************************/
std::shared_ptr<QtNodes::NodeData> SIFTFeaturesModel::outData(QtNodes::PortIndex port)
{
    if (port == 0)
    {
        return m_outputImage;
    }
    else
    {
        return m_keypointData;
    }
}

void SIFTFeaturesModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                  QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    detectFeatures();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* SIFTFeaturesModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void SIFTFeaturesModel::onFeaturesChanged(int value)
{
    m_nFeatures = value;
    detectFeatures();
}

void SIFTFeaturesModel::onOctaveLayersChanged(int value)
{
    m_nOctaveLayers = value;
    detectFeatures();
}

void SIFTFeaturesModel::onContrastThresholdChanged(double value)
{
    m_contrastThreshold = value;
    detectFeatures();
}

void SIFTFeaturesModel::onEdgeThresholdChanged(double value)
{
    m_edgeThreshold = value;
    detectFeatures();
}

void SIFTFeaturesModel::onSigmaChanged(double value)
{
    m_sigma = value;
    detectFeatures();
}

void SIFTFeaturesModel::onShowKeypointsChanged(int state)
{
    m_showKeypoints = (state == Qt::Checked);
    detectFeatures();
}

/*******************************************************************************
 * Feature Detection
 ******************************************************************************/
void SIFTFeaturesModel::detectFeatures()
{
    if (!m_inputImage)
    {
        m_outputImage = nullptr;
        m_keypointData = nullptr;
        m_keypointsLabel->setText("Keypoints: 0");
        Q_EMIT dataUpdated(0);
        Q_EMIT dataUpdated(1);
        return;
    }

    cv::Mat input = m_inputImage->image();

    if (input.empty())
    {
        m_outputImage = nullptr;
        m_keypointData = nullptr;
        m_keypointsLabel->setText("Keypoints: 0");
        Q_EMIT dataUpdated(0);
        Q_EMIT dataUpdated(1);
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

        // Recreate SIFT detector with new parameters
        m_sift = cv::SIFT::create(
            m_nFeatures,
            m_nOctaveLayers,
            m_contrastThreshold,
            m_edgeThreshold,
            m_sigma
        );

        // Detect keypoints
        std::vector<cv::KeyPoint> keypoints;
        m_sift->detectAndCompute(gray, cv::noArray(), keypoints, cv::noArray());

        // Update keypoints label
        m_keypointsLabel->setText(QString("Keypoints: %1").arg(keypoints.size()));

        // Create keypoint data
        m_keypointData = std::make_shared<KeypointData>(keypoints);

        // Create visualization
        if (m_showKeypoints)
        {
            cv::Mat output;
            cv::drawKeypoints(
                input,
                keypoints,
                output,
                cv::Scalar::all(-1),
                cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS
            );
            m_outputImage = std::make_shared<ImageData>(output);
        }
        else
        {
            m_outputImage = std::make_shared<ImageData>(input);
        }

        Q_EMIT dataUpdated(0);
        Q_EMIT dataUpdated(1);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        m_keypointData = nullptr;
        m_keypointsLabel->setText("Keypoints: Error");
        Q_EMIT dataUpdated(0);
        Q_EMIT dataUpdated(1);
    }
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject SIFTFeaturesModel::save() const
{
    QJsonObject modelJson;
    modelJson["nFeatures"] = m_nFeatures;
    modelJson["nOctaveLayers"] = m_nOctaveLayers;
    modelJson["contrastThreshold"] = m_contrastThreshold;
    modelJson["edgeThreshold"] = m_edgeThreshold;
    modelJson["sigma"] = m_sigma;
    modelJson["showKeypoints"] = m_showKeypoints;
    return modelJson;
}

void SIFTFeaturesModel::load(QJsonObject const& model)
{
    QJsonValue featuresJson = model["nFeatures"];
    if (!featuresJson.isUndefined())
    {
        m_nFeatures = featuresJson.toInt();
        m_featuresSpin->setValue(m_nFeatures);
    }

    QJsonValue octaveLayersJson = model["nOctaveLayers"];
    if (!octaveLayersJson.isUndefined())
    {
        m_nOctaveLayers = octaveLayersJson.toInt();
        m_octaveLayersSpin->setValue(m_nOctaveLayers);
    }

    QJsonValue contrastThresholdJson = model["contrastThreshold"];
    if (!contrastThresholdJson.isUndefined())
    {
        m_contrastThreshold = contrastThresholdJson.toDouble();
        m_contrastThresholdSpin->setValue(m_contrastThreshold);
    }

    QJsonValue edgeThresholdJson = model["edgeThreshold"];
    if (!edgeThresholdJson.isUndefined())
    {
        m_edgeThreshold = edgeThresholdJson.toDouble();
        m_edgeThresholdSpin->setValue(m_edgeThreshold);
    }

    QJsonValue sigmaJson = model["sigma"];
    if (!sigmaJson.isUndefined())
    {
        m_sigma = sigmaJson.toDouble();
        m_sigmaSpin->setValue(m_sigma);
    }

    QJsonValue showKeypointsJson = model["showKeypoints"];
    if (!showKeypointsJson.isUndefined())
    {
        m_showKeypoints = showKeypointsJson.toBool();
        m_showKeypointsCheck->setChecked(m_showKeypoints);
    }

    detectFeatures();
}

} // namespace VisionBox
