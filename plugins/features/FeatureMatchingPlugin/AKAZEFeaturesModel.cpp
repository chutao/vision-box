/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * AKAZE Features Model Implementation
 ******************************************************************************/

#include "AKAZEFeaturesModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
AKAZEFeaturesModel::AKAZEFeaturesModel()
    : m_outputImage(nullptr)
    , m_keypointData(nullptr)
{
    // Create AKAZE detector
    m_akaze = cv::AKAZE::create(
        static_cast<cv::AKAZE::DescriptorType>(m_descriptorType),
        m_descriptorSize,
        m_descriptorChannels,
        m_threshold,
        m_nOctaves,
        m_nOctaveLayers,
        static_cast<cv::KAZE::DiffusivityType>(m_diffusivity)
    );

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Descriptor type
    auto* descTypeLayout = new QHBoxLayout();
    descTypeLayout->addWidget(new QLabel("Descriptor:"));
    m_descriptorTypeCombo = new QComboBox();
    m_descriptorTypeCombo->addItem("MLDB", cv::AKAZE::DESCRIPTOR_MLDB);
    m_descriptorTypeCombo->addItem("MLDB Upright", cv::AKAZE::DESCRIPTOR_MLDB_UPRIGHT);
    m_descriptorTypeCombo->setCurrentIndex(0);
    m_descriptorTypeCombo->setMinimumWidth(150);
    descTypeLayout->addWidget(m_descriptorTypeCombo);
    layout->addLayout(descTypeLayout);

    // Descriptor size
    auto* descSizeLayout = new QHBoxLayout();
    descSizeLayout->addWidget(new QLabel("Desc Size:"));
    m_descriptorSizeSpin = new QSpinBox();
    m_descriptorSizeSpin->setRange(0, 6);  // 0 = full size (486 bits for MLDB)
    m_descriptorSizeSpin->setValue(0);
    m_descriptorSizeSpin->setToolTip("0 = full size");
    descSizeLayout->addWidget(m_descriptorSizeSpin);
    layout->addLayout(descSizeLayout);

    // Descriptor channels
    auto* descChannelsLayout = new QHBoxLayout();
    descChannelsLayout->addWidget(new QLabel("Channels:"));
    m_descriptorChannelsSpin = new QSpinBox();
    m_descriptorChannelsSpin->setRange(1, 3);
    m_descriptorChannelsSpin->setValue(3);
    descChannelsLayout->addWidget(m_descriptorChannelsSpin);
    layout->addLayout(descChannelsLayout);

    // Threshold
    auto* thresholdLayout = new QHBoxLayout();
    thresholdLayout->addWidget(new QLabel("Threshold:"));
    m_thresholdSpin = new QDoubleSpinBox();
    m_thresholdSpin->setRange(0.0001, 1.0);
    m_thresholdSpin->setDecimals(4);
    m_thresholdSpin->setSingleStep(0.0001);
    m_thresholdSpin->setValue(0.001);
    m_thresholdSpin->setToolTip("Detector response threshold");
    thresholdLayout->addWidget(m_thresholdSpin);
    layout->addLayout(thresholdLayout);

    // Octaves
    auto* octavesLayout = new QHBoxLayout();
    octavesLayout->addWidget(new QLabel("Octaves:"));
    m_nOctavesSpin = new QSpinBox();
    m_nOctavesSpin->setRange(1, 10);
    m_nOctavesSpin->setValue(4);
    octavesLayout->addWidget(m_nOctavesSpin);
    layout->addLayout(octavesLayout);

    // Octave layers
    auto* octaveLayersLayout = new QHBoxLayout();
    octaveLayersLayout->addWidget(new QLabel("Octave Layers:"));
    m_nOctaveLayersSpin = new QSpinBox();
    m_nOctaveLayersSpin->setRange(1, 10);
    m_nOctaveLayersSpin->setValue(4);
    octaveLayersLayout->addWidget(m_nOctaveLayersSpin);
    layout->addLayout(octaveLayersLayout);

    // Diffusivity
    auto* diffusivityLayout = new QHBoxLayout();
    diffusivityLayout->addWidget(new QLabel("Diffusivity:"));
    m_diffusivityCombo = new QComboBox();
    m_diffusivityCombo->addItem("PM G2", cv::KAZE::DIFF_PM_G2);
    m_diffusivityCombo->addItem("PM G1", cv::KAZE::DIFF_PM_G1);
    m_diffusivityCombo->addItem("Charbonnier", cv::KAZE::DIFF_CHARBONNIER);
    m_diffusivityCombo->setCurrentIndex(0);
    m_diffusivityCombo->setMinimumWidth(150);
    diffusivityLayout->addWidget(m_diffusivityCombo);
    layout->addLayout(diffusivityLayout);

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
    connect(m_descriptorTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AKAZEFeaturesModel::onDescriptorTypeChanged);
    connect(m_descriptorSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AKAZEFeaturesModel::onDescriptorSizeChanged);
    connect(m_descriptorChannelsSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AKAZEFeaturesModel::onDescriptorChannelsChanged);
    connect(m_thresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AKAZEFeaturesModel::onThresholdChanged);
    connect(m_nOctavesSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AKAZEFeaturesModel::onNOctavesChanged);
    connect(m_nOctaveLayersSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AKAZEFeaturesModel::onNOctaveLayersChanged);
    connect(m_diffusivityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AKAZEFeaturesModel::onDiffusivityChanged);
    connect(m_showKeypointsCheck, &QCheckBox::stateChanged,
            this, &AKAZEFeaturesModel::onShowKeypointsChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int AKAZEFeaturesModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType AKAZEFeaturesModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> AKAZEFeaturesModel::outData(QtNodes::PortIndex port)
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

void AKAZEFeaturesModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                   QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    detectFeatures();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* AKAZEFeaturesModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void AKAZEFeaturesModel::onDescriptorTypeChanged()
{
    m_descriptorType = m_descriptorTypeCombo->currentData().toInt();
    detectFeatures();
}

void AKAZEFeaturesModel::onDescriptorSizeChanged(int value)
{
    m_descriptorSize = value;
    detectFeatures();
}

void AKAZEFeaturesModel::onDescriptorChannelsChanged(int value)
{
    m_descriptorChannels = value;
    detectFeatures();
}

void AKAZEFeaturesModel::onThresholdChanged(double value)
{
    m_threshold = value;
    detectFeatures();
}

void AKAZEFeaturesModel::onNOctavesChanged(int value)
{
    m_nOctaves = value;
    detectFeatures();
}

void AKAZEFeaturesModel::onNOctaveLayersChanged(int value)
{
    m_nOctaveLayers = value;
    detectFeatures();
}

void AKAZEFeaturesModel::onDiffusivityChanged()
{
    m_diffusivity = m_diffusivityCombo->currentData().toInt();
    detectFeatures();
}

void AKAZEFeaturesModel::onShowKeypointsChanged(int state)
{
    m_showKeypoints = (state == Qt::Checked);
    detectFeatures();
}

/*******************************************************************************
 * Feature Detection
 ******************************************************************************/
void AKAZEFeaturesModel::detectFeatures()
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

        // Recreate AKAZE detector with new parameters
        m_akaze = cv::AKAZE::create(
            static_cast<cv::AKAZE::DescriptorType>(m_descriptorType),
            m_descriptorSize,
            m_descriptorChannels,
            m_threshold,
            m_nOctaves,
            m_nOctaveLayers,
            static_cast<cv::KAZE::DiffusivityType>(m_diffusivity)
        );

        // Detect keypoints
        std::vector<cv::KeyPoint> keypoints;
        m_akaze->detectAndCompute(gray, cv::noArray(), keypoints, cv::noArray());

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
QJsonObject AKAZEFeaturesModel::save() const
{
    QJsonObject modelJson;
    modelJson["descriptorType"] = m_descriptorType;
    modelJson["descriptorSize"] = m_descriptorSize;
    modelJson["descriptorChannels"] = m_descriptorChannels;
    modelJson["threshold"] = m_threshold;
    modelJson["nOctaves"] = m_nOctaves;
    modelJson["nOctaveLayers"] = m_nOctaveLayers;
    modelJson["diffusivity"] = m_diffusivity;
    modelJson["showKeypoints"] = m_showKeypoints;
    return modelJson;
}

void AKAZEFeaturesModel::load(QJsonObject const& model)
{
    QJsonValue descriptorTypeJson = model["descriptorType"];
    if (!descriptorTypeJson.isUndefined())
    {
        m_descriptorType = descriptorTypeJson.toInt();
        for (int i = 0; i < m_descriptorTypeCombo->count(); ++i)
        {
            if (m_descriptorTypeCombo->itemData(i).toInt() == m_descriptorType)
            {
                m_descriptorTypeCombo->blockSignals(true);
                m_descriptorTypeCombo->setCurrentIndex(i);
                m_descriptorTypeCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue descriptorSizeJson = model["descriptorSize"];
    if (!descriptorSizeJson.isUndefined())
    {
        m_descriptorSize = descriptorSizeJson.toInt();
        m_descriptorSizeSpin->setValue(m_descriptorSize);
    }

    QJsonValue descriptorChannelsJson = model["descriptorChannels"];
    if (!descriptorChannelsJson.isUndefined())
    {
        m_descriptorChannels = descriptorChannelsJson.toInt();
        m_descriptorChannelsSpin->setValue(m_descriptorChannels);
    }

    QJsonValue thresholdJson = model["threshold"];
    if (!thresholdJson.isUndefined())
    {
        m_threshold = thresholdJson.toDouble();
        m_thresholdSpin->setValue(m_threshold);
    }

    QJsonValue nOctavesJson = model["nOctaves"];
    if (!nOctavesJson.isUndefined())
    {
        m_nOctaves = nOctavesJson.toInt();
        m_nOctavesSpin->setValue(m_nOctaves);
    }

    QJsonValue nOctaveLayersJson = model["nOctaveLayers"];
    if (!nOctaveLayersJson.isUndefined())
    {
        m_nOctaveLayers = nOctaveLayersJson.toInt();
        m_nOctaveLayersSpin->setValue(m_nOctaveLayers);
    }

    QJsonValue diffusivityJson = model["diffusivity"];
    if (!diffusivityJson.isUndefined())
    {
        m_diffusivity = diffusivityJson.toInt();
        for (int i = 0; i < m_diffusivityCombo->count(); ++i)
        {
            if (m_diffusivityCombo->itemData(i).toInt() == m_diffusivity)
            {
                m_diffusivityCombo->blockSignals(true);
                m_diffusivityCombo->setCurrentIndex(i);
                m_diffusivityCombo->blockSignals(false);
                break;
            }
        }
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
