/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * ORB Feature Model Implementation
 ******************************************************************************/

#include "ORBFeatureModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ORBFeatureModel::ORBFeatureModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Max features
    auto* maxFeatLayout = new QHBoxLayout();
    maxFeatLayout->addWidget(new QLabel("Max Features:"));
    m_maxFeaturesSpin = new QSpinBox();
    m_maxFeaturesSpin->setRange(50, 10000);
    m_maxFeaturesSpin->setValue(500);
    maxFeatLayout->addWidget(m_maxFeaturesSpin);
    layout->addLayout(maxFeatLayout);

    // Scale factor
    auto* scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("Scale Factor:"));
    m_scaleFactorSpin = new QDoubleSpinBox();
    m_scaleFactorSpin->setRange(1.0, 2.0);
    m_scaleFactorSpin->setSingleStep(0.1);
    m_scaleFactorSpin->setDecimals(2);
    m_scaleFactorSpin->setValue(1.2);
    scaleLayout->addWidget(m_scaleFactorSpin);
    layout->addLayout(scaleLayout);

    // Number of levels
    auto* nLevelsLayout = new QHBoxLayout();
    nLevelsLayout->addWidget(new QLabel("Pyramid Levels:"));
    m_nLevelsSpin = new QSpinBox();
    m_nLevelsSpin->setRange(1, 20);
    m_nLevelsSpin->setValue(8);
    nLevelsLayout->addWidget(m_nLevelsSpin);
    layout->addLayout(nLevelsLayout);

    // Edge threshold
    auto* edgeLayout = new QHBoxLayout();
    edgeLayout->addWidget(new QLabel("Edge Threshold:"));
    m_edgeThresholdSpin = new QSpinBox();
    m_edgeThresholdSpin->setRange(0, 100);
    m_edgeThresholdSpin->setValue(31);
    edgeLayout->addWidget(m_edgeThresholdSpin);
    layout->addLayout(edgeLayout);

    // First level
    auto* firstLayout = new QHBoxLayout();
    firstLayout->addWidget(new QLabel("First Level:"));
    m_firstLevelSpin = new QSpinBox();
    m_firstLevelSpin->setRange(0, 10);
    m_firstLevelSpin->setValue(0);
    firstLayout->addWidget(m_firstLevelSpin);
    layout->addLayout(firstLayout);

    // WTA K
    auto* wtaLayout = new QHBoxLayout();
    wtaLayout->addWidget(new QLabel("WTA K:"));
    m_wtaKSpin = new QSpinBox();
    m_wtaKSpin->setRange(2, 4);
    m_wtaKSpin->setValue(2);
    wtaLayout->addWidget(m_wtaKSpin);
    layout->addLayout(wtaLayout);

    // Score type
    auto* scoreLayout = new QHBoxLayout();
    scoreLayout->addWidget(new QLabel("Score Type:"));
    m_scoreTypeCombo = new QComboBox();
    m_scoreTypeCombo->addItem("Harris Score", 0);
    m_scoreTypeCombo->addItem("FAST Score", 1);
    scoreLayout->addWidget(m_scoreTypeCombo);
    layout->addLayout(scoreLayout);

    // Patch size
    auto* patchLayout = new QHBoxLayout();
    patchLayout->addWidget(new QLabel("Patch Size:"));
    m_patchSizeSpin = new QSpinBox();
    m_patchSizeSpin->setRange(1, 100);
    m_patchSizeSpin->setSingleStep(2);
    m_patchSizeSpin->setValue(31);
    patchLayout->addWidget(m_patchSizeSpin);
    layout->addLayout(patchLayout);

    // FAST threshold
    auto* fastLayout = new QHBoxLayout();
    fastLayout->addWidget(new QLabel("FAST Threshold:"));
    m_fastThresholdSpin = new QSpinBox();
    m_fastThresholdSpin->setRange(0, 100);
    m_fastThresholdSpin->setValue(20);
    fastLayout->addWidget(m_fastThresholdSpin);
    layout->addLayout(fastLayout);

    // Draw keypoints checkbox
    m_drawKeypointsCheck = new QCheckBox("Draw Keypoints");
    m_drawKeypointsCheck->setChecked(true);
    layout->addWidget(m_drawKeypointsCheck);

    // Draw rich keypoints checkbox
    m_drawRichCheck = new QCheckBox("Draw Rich Keypoints");
    m_drawRichCheck->setChecked(true);
    layout->addWidget(m_drawRichCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_maxFeaturesSpin, &QSpinBox::valueChanged,
            this, &ORBFeatureModel::onMaxFeaturesChanged);
    connect(m_scaleFactorSpin, &QDoubleSpinBox::valueChanged,
            this, &ORBFeatureModel::onScaleFactorChanged);
    connect(m_nLevelsSpin, &QSpinBox::valueChanged,
            this, &ORBFeatureModel::onNLevelsChanged);
    connect(m_edgeThresholdSpin, &QSpinBox::valueChanged,
            this, &ORBFeatureModel::onEdgeThresholdChanged);
    connect(m_firstLevelSpin, &QSpinBox::valueChanged,
            this, &ORBFeatureModel::onFirstLevelChanged);
    connect(m_wtaKSpin, &QSpinBox::valueChanged,
            this, &ORBFeatureModel::onWTAKChanged);
    connect(m_scoreTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ORBFeatureModel::onScoreTypeChanged);
    connect(m_patchSizeSpin, &QSpinBox::valueChanged,
            this, &ORBFeatureModel::onPatchSizeChanged);
    connect(m_fastThresholdSpin, &QSpinBox::valueChanged,
            this, &ORBFeatureModel::onFastThresholdChanged);
    connect(m_drawKeypointsCheck, &QCheckBox::stateChanged,
            this, &ORBFeatureModel::onDrawKeypointsChanged);
    connect(m_drawRichCheck, &QCheckBox::stateChanged,
            this, &ORBFeatureModel::onDrawRichChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ORBFeatureModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ORBFeatureModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ORBFeatureModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ORBFeatureModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    detectFeatures();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ORBFeatureModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ORBFeatureModel::onMaxFeaturesChanged(int value)
{
    m_maxFeatures = value;
    detectFeatures();
}

void ORBFeatureModel::onScaleFactorChanged(double value)
{
    m_scaleFactor = value;
    detectFeatures();
}

void ORBFeatureModel::onNLevelsChanged(int value)
{
    m_nLevels = value;
    detectFeatures();
}

void ORBFeatureModel::onEdgeThresholdChanged(int value)
{
    m_edgeThreshold = value;
    detectFeatures();
}

void ORBFeatureModel::onFirstLevelChanged(int value)
{
    m_firstLevel = value;
    detectFeatures();
}

void ORBFeatureModel::onWTAKChanged(int value)
{
    m_wtaK = value;
    detectFeatures();
}

void ORBFeatureModel::onScoreTypeChanged(int index)
{
    m_scoreType = m_scoreTypeCombo->itemData(index).toInt();
    detectFeatures();
}

void ORBFeatureModel::onPatchSizeChanged(int value)
{
    m_patchSize = value;
    detectFeatures();
}

void ORBFeatureModel::onFastThresholdChanged(int value)
{
    m_fastThreshold = value;
    detectFeatures();
}

void ORBFeatureModel::onDrawKeypointsChanged(int state)
{
    m_drawKeypoints = (state == Qt::Checked);
    detectFeatures();
}

void ORBFeatureModel::onDrawRichChanged(int state)
{
    m_drawRich = (state == Qt::Checked);
    detectFeatures();
}

/*******************************************************************************
 * Feature Detection
 ******************************************************************************/
void ORBFeatureModel::detectFeatures()
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
        // Create ORB detector with current parameters
        cv::Ptr<cv::ORB> orb = cv::ORB::create(
            m_maxFeatures,
            m_scaleFactor,
            m_nLevels,
            m_edgeThreshold,
            m_firstLevel,
            m_wtaK,
            m_scoreType == 0 ? cv::ORB::HARRIS_SCORE : cv::ORB::FAST_SCORE,
            m_patchSize,
            m_fastThreshold
        );

        // Detect keypoints and compute descriptors
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;

        cv::Mat gray;
        if (input.channels() > 1)
        {
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = input;
        }

        orb->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);

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

        // Draw keypoints
        if (m_drawKeypoints && !keypoints.empty())
        {
            if (m_drawRich)
            {
                // Draw rich keypoints (circles with orientation)
                cv::drawKeypoints(output, keypoints, output,
                                 cv::Scalar(0, 255, 0),  // Green in BGR
                                 cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            }
            else
            {
                // Draw simple keypoints
                cv::drawKeypoints(output, keypoints, output,
                                 cv::Scalar(0, 255, 0),  // Green in BGR
                                 cv::DrawMatchesFlags::DEFAULT);
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
QJsonObject ORBFeatureModel::save() const
{
    QJsonObject modelJson;
    modelJson["maxFeatures"] = m_maxFeatures;
    modelJson["scaleFactor"] = m_scaleFactor;
    modelJson["nLevels"] = m_nLevels;
    modelJson["edgeThreshold"] = m_edgeThreshold;
    modelJson["firstLevel"] = m_firstLevel;
    modelJson["wtaK"] = m_wtaK;
    modelJson["scoreType"] = m_scoreType;
    modelJson["patchSize"] = m_patchSize;
    modelJson["fastThreshold"] = m_fastThreshold;
    modelJson["drawKeypoints"] = m_drawKeypoints;
    modelJson["drawRich"] = m_drawRich;
    return modelJson;
}

void ORBFeatureModel::load(QJsonObject const& model)
{
    QJsonValue maxFeatJson = model["maxFeatures"];
    if (!maxFeatJson.isUndefined())
    {
        m_maxFeatures = maxFeatJson.toInt();
        m_maxFeaturesSpin->setValue(m_maxFeatures);
    }

    QJsonValue scaleJson = model["scaleFactor"];
    if (!scaleJson.isUndefined())
    {
        m_scaleFactor = scaleJson.toDouble();
        m_scaleFactorSpin->setValue(m_scaleFactor);
    }

    QJsonValue nLevelsJson = model["nLevels"];
    if (!nLevelsJson.isUndefined())
    {
        m_nLevels = nLevelsJson.toInt();
        m_nLevelsSpin->setValue(m_nLevels);
    }

    QJsonValue edgeJson = model["edgeThreshold"];
    if (!edgeJson.isUndefined())
    {
        m_edgeThreshold = edgeJson.toInt();
        m_edgeThresholdSpin->setValue(m_edgeThreshold);
    }

    QJsonValue firstJson = model["firstLevel"];
    if (!firstJson.isUndefined())
    {
        m_firstLevel = firstJson.toInt();
        m_firstLevelSpin->setValue(m_firstLevel);
    }

    QJsonValue wtaJson = model["wtaK"];
    if (!wtaJson.isUndefined())
    {
        m_wtaK = wtaJson.toInt();
        m_wtaKSpin->setValue(m_wtaK);
    }

    QJsonValue scoreJson = model["scoreType"];
    if (!scoreJson.isUndefined())
    {
        m_scoreType = scoreJson.toInt();
        m_scoreTypeCombo->setCurrentIndex(m_scoreType);
    }

    QJsonValue patchJson = model["patchSize"];
    if (!patchJson.isUndefined())
    {
        m_patchSize = patchJson.toInt();
        m_patchSizeSpin->setValue(m_patchSize);
    }

    QJsonValue fastJson = model["fastThreshold"];
    if (!fastJson.isUndefined())
    {
        m_fastThreshold = fastJson.toInt();
        m_fastThresholdSpin->setValue(m_fastThreshold);
    }

    QJsonValue drawJson = model["drawKeypoints"];
    if (!drawJson.isUndefined())
    {
        m_drawKeypoints = drawJson.toBool();
        m_drawKeypointsCheck->setChecked(m_drawKeypoints);
    }

    QJsonValue richJson = model["drawRich"];
    if (!richJson.isUndefined())
    {
        m_drawRich = richJson.toBool();
        m_drawRichCheck->setChecked(m_drawRich);
    }

    detectFeatures();
}

} // namespace VisionBox
