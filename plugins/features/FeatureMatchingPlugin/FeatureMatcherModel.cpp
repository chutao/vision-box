/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Matcher Model Implementation
 ******************************************************************************/

#include "FeatureMatcherModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
FeatureMatcherModel::FeatureMatcherModel()
    : m_outputImage(nullptr)
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Matcher type
    auto* matcherLayout = new QHBoxLayout();
    matcherLayout->addWidget(new QLabel("Matcher:"));
    m_matcherCombo = new QComboBox();
    m_matcherCombo->addItem("Brute Force", BruteForce);
    m_matcherCombo->addItem("Brute Force (SL2)", BruteForceSL2);
    m_matcherCombo->addItem("Brute Force (L1)", BruteForceL1);
    m_matcherCombo->addItem("Brute Force (Hamming)", BruteForceHamming);
    m_matcherCombo->addItem("Brute Force (HammingLUT)", BruteForceHammingLUT);
    m_matcherCombo->addItem("FLANN Based", FlannBased);
    m_matcherCombo->setCurrentIndex(5); // FLANN Based
    m_matcherCombo->setMinimumWidth(150);
    matcherLayout->addWidget(m_matcherCombo);
    layout->addLayout(matcherLayout);

    // Norm type (for BruteForce)
    auto* normLayout = new QHBoxLayout();
    normLayout->addWidget(new QLabel("Norm Type:"));
    m_normTypeCombo = new QComboBox();
    m_normTypeCombo->addItem("L2", cv::NORM_L2);
    m_normTypeCombo->addItem("L1", cv::NORM_L1);
    m_normTypeCombo->addItem("HAMMING", cv::NORM_HAMMING);
    m_normTypeCombo->addItem("HAMMING2", cv::NORM_HAMMING2);
    m_normTypeCombo->setCurrentIndex(0);
    m_normTypeCombo->setEnabled(false);
    m_normTypeCombo->setMinimumWidth(120);
    normLayout->addWidget(m_normTypeCombo);
    layout->addLayout(normLayout);

    // Cross-check
    m_crossCheckCheck = new QCheckBox("Cross-Check");
    m_crossCheckCheck->setToolTip("Use cross-check filtering for better matches");
    m_crossCheckCheck->setChecked(false);
    layout->addWidget(m_crossCheckCheck);

    // Max ratio (Lowe's ratio test)
    auto* ratioLayout = new QHBoxLayout();
    ratioLayout->addWidget(new QLabel("Max Ratio:"));
    m_maxRatioSpin = new QDoubleSpinBox();
    m_maxRatioSpin->setRange(0.1, 1.0);
    m_maxRatioSpin->setSingleStep(0.05);
    m_maxRatioSpin->setValue(0.75);
    m_maxRatioSpin->setToolTip("Lowe's ratio test threshold");
    ratioLayout->addWidget(m_maxRatioSpin);
    layout->addLayout(ratioLayout);

    // Max distance
    auto* distanceLayout = new QHBoxLayout();
    distanceLayout->addWidget(new QLabel("Max Distance:"));
    m_maxDistanceSpin = new QDoubleSpinBox();
    m_maxDistanceSpin->setRange(0.0, 1000.0);
    m_maxDistanceSpin->setSingleStep(10.0);
    m_maxDistanceSpin->setValue(100.0);
    distanceLayout->addWidget(m_maxDistanceSpin);
    layout->addLayout(distanceLayout);

    // Max matches
    auto* matchesLayout = new QHBoxLayout();
    matchesLayout->addWidget(new QLabel("Max Matches:"));
    m_maxMatchesSpin = new QSpinBox();
    m_maxMatchesSpin->setRange(1, 1000);
    m_maxMatchesSpin->setValue(100);
    matchesLayout->addWidget(m_maxMatchesSpin);
    layout->addLayout(matchesLayout);

    // Matches count label
    m_matchesLabel = new QLabel("Matches: 0");
    m_matchesLabel->setStyleSheet("QLabel { font-weight: bold; padding: 5px; }");
    layout->addWidget(m_matchesLabel);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_matcherCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FeatureMatcherModel::onMatcherChanged);
    connect(m_normTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FeatureMatcherModel::onNormTypeChanged);
    connect(m_crossCheckCheck, &QCheckBox::stateChanged,
            this, &FeatureMatcherModel::onCrossCheckChanged);
    connect(m_maxRatioSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FeatureMatcherModel::onMaxRatioChanged);
    connect(m_maxDistanceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FeatureMatcherModel::onMaxDistanceChanged);
    connect(m_maxMatchesSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FeatureMatcherModel::onMaxMatchesChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int FeatureMatcherModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 4; // Image1, Image2, Keypoints1, Keypoints2
    }
    else
    {
        return 1; // Match visualization
    }
}

QtNodes::NodeDataType FeatureMatcherModel::dataType(
    QtNodes::PortType portType,
    QtNodes::PortIndex portIndex) const
{
    if (portType == QtNodes::PortType::In)
    {
        if (portIndex == 0 || portIndex == 1)
        {
            return ImageData().type(); // Images
        }
        else
        {
            return KeypointData().type(); // Keypoints
        }
    }
    return ImageData().type(); // Output
}

/*******************************************************************************
 * Data Flow
 ******************************************************************************/
std::shared_ptr<QtNodes::NodeData> FeatureMatcherModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void FeatureMatcherModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                    QtNodes::PortIndex portIndex)
{
    if (portIndex == 0)
    {
        m_inputImage1 = std::dynamic_pointer_cast<ImageData>(data);
    }
    else if (portIndex == 1)
    {
        m_inputImage2 = std::dynamic_pointer_cast<ImageData>(data);
    }
    else if (portIndex == 2)
    {
        m_keypoints1 = std::dynamic_pointer_cast<KeypointData>(data);
    }
    else if (portIndex == 3)
    {
        m_keypoints2 = std::dynamic_pointer_cast<KeypointData>(data);
    }

    matchFeatures();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* FeatureMatcherModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void FeatureMatcherModel::onMatcherChanged()
{
    m_matcherType = static_cast<MatcherType>(m_matcherCombo->currentData().toInt());

    // Enable/disable norm type based on matcher
    bool useNormType = (m_matcherType == BruteForce);
    m_normTypeCombo->setEnabled(useNormType);

    matchFeatures();
}

void FeatureMatcherModel::onNormTypeChanged()
{
    m_normType = m_normTypeCombo->currentData().toInt();
    matchFeatures();
}

void FeatureMatcherModel::onCrossCheckChanged(int state)
{
    m_crossCheck = (state == Qt::Checked);
    matchFeatures();
}

void FeatureMatcherModel::onMaxRatioChanged(double value)
{
    m_maxRatio = value;
    matchFeatures();
}

void FeatureMatcherModel::onMaxDistanceChanged(double value)
{
    m_maxDistance = value;
    matchFeatures();
}

void FeatureMatcherModel::onMaxMatchesChanged(int value)
{
    m_maxMatches = value;
    matchFeatures();
}

/*******************************************************************************
 * Feature Matching
 ******************************************************************************/
cv::Ptr<cv::DescriptorMatcher> FeatureMatcherModel::createMatcher()
{
    switch (m_matcherType)
    {
        case BruteForce:
            return cv::BFMatcher::create(m_normType, m_crossCheck);

        case BruteForceSL2:
            return cv::BFMatcher::create(cv::NORM_L2, m_crossCheck);

        case BruteForceL1:
            return cv::BFMatcher::create(cv::NORM_L1, m_crossCheck);

        case BruteForceHamming:
            return cv::BFMatcher::create(cv::NORM_HAMMING, m_crossCheck);

        case BruteForceHammingLUT:
            return cv::BFMatcher::create(cv::NORM_HAMMING2, m_crossCheck);

        case FlannBased:
        default:
            return cv::FlannBasedMatcher::create();
    }
}

void FeatureMatcherModel::matchFeatures()
{
    // Check if we have all required inputs
    if (!m_inputImage1 || !m_inputImage2 || !m_keypoints1 || !m_keypoints2)
    {
        m_outputImage = nullptr;
        m_matchesLabel->setText("Matches: Need 2 images + 2 keypoints");
        Q_EMIT dataUpdated(0);
        return;
    }

    cv::Mat image1 = m_inputImage1->image();
    cv::Mat image2 = m_inputImage2->image();

    if (image1.empty() || image2.empty())
    {
        m_outputImage = nullptr;
        m_matchesLabel->setText("Matches: Invalid images");
        Q_EMIT dataUpdated(0);
        return;
    }

    std::vector<cv::KeyPoint> keypoints1 = m_keypoints1->keypoints();
    std::vector<cv::KeyPoint> keypoints2 = m_keypoints2->keypoints();

    if (keypoints1.empty() || keypoints2.empty())
    {
        m_outputImage = nullptr;
        m_matchesLabel->setText("Matches: No keypoints");
        Q_EMIT dataUpdated(0);
        return;
    }

    try
    {
        // Convert to grayscale for descriptor computation
        cv::Mat gray1, gray2;
        if (image1.channels() > 1)
        {
            cv::cvtColor(image1, gray1, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray1 = image1;
        }

        if (image2.channels() > 1)
        {
            cv::cvtColor(image2, gray2, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray2 = image2;
        }

        // Compute descriptors using ORB (works with all matcher types)
        cv::Ptr<cv::ORB> orb = cv::ORB::create();
        cv::Mat descriptors1, descriptors2;
        orb->compute(gray1, keypoints1, descriptors1);
        orb->compute(gray2, keypoints2, descriptors2);

        if (descriptors1.empty() || descriptors2.empty())
        {
            m_outputImage = nullptr;
            m_matchesLabel->setText("Matches: No descriptors");
            Q_EMIT dataUpdated(0);
            return;
        }

        // Create matcher
        cv::Ptr<cv::DescriptorMatcher> matcher = createMatcher();

        // Match descriptors
        std::vector<std::vector<cv::DMatch>> knnMatches;
        matcher->knnMatch(descriptors1, descriptors2, knnMatches, 2);

        // Apply Lowe's ratio test
        std::vector<cv::DMatch> goodMatches;
        for (size_t i = 0; i < knnMatches.size(); ++i)
        {
            if (knnMatches[i].size() >= 2)
            {
                if (knnMatches[i][0].distance < m_maxRatio * knnMatches[i][1].distance)
                {
                    goodMatches.push_back(knnMatches[i][0]);
                }
            }
            else if (knnMatches[i].size() == 1)
            {
                // Only one match found
                if (knnMatches[i][0].distance < m_maxDistance)
                {
                    goodMatches.push_back(knnMatches[i][0]);
                }
            }
        }

        // Limit to max matches
        if (goodMatches.size() > static_cast<size_t>(m_maxMatches))
        {
            std::sort(goodMatches.begin(), goodMatches.end());
            goodMatches.resize(m_maxMatches);
        }

        // Update matches label
        m_matchesLabel->setText(QString("Matches: %1").arg(goodMatches.size()));

        // Draw matches
        cv::Mat output;
        cv::drawMatches(
            image1,
            keypoints1,
            image2,
            keypoints2,
            goodMatches,
            output,
            cv::Scalar::all(-1),
            cv::Scalar::all(-1),
            std::vector<char>(),
            cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS
        );

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        m_matchesLabel->setText("Matches: Error");
        Q_EMIT dataUpdated(0);
    }
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject FeatureMatcherModel::save() const
{
    QJsonObject modelJson;
    modelJson["matcherType"] = static_cast<int>(m_matcherType);
    modelJson["normType"] = m_normType;
    modelJson["crossCheck"] = m_crossCheck;
    modelJson["maxRatio"] = m_maxRatio;
    modelJson["maxDistance"] = m_maxDistance;
    modelJson["maxMatches"] = m_maxMatches;
    return modelJson;
}

void FeatureMatcherModel::load(QJsonObject const& model)
{
    QJsonValue matcherTypeJson = model["matcherType"];
    if (!matcherTypeJson.isUndefined())
    {
        int matcherType = matcherTypeJson.toInt();
        for (int i = 0; i < m_matcherCombo->count(); ++i)
        {
            if (m_matcherCombo->itemData(i).toInt() == matcherType)
            {
                m_matcherCombo->blockSignals(true);
                m_matcherCombo->setCurrentIndex(i);
                m_matcherCombo->blockSignals(false);
                break;
            }
        }
        onMatcherChanged();
    }

    QJsonValue normTypeJson = model["normType"];
    if (!normTypeJson.isUndefined())
    {
        m_normType = normTypeJson.toInt();
        for (int i = 0; i < m_normTypeCombo->count(); ++i)
        {
            if (m_normTypeCombo->itemData(i).toInt() == m_normType)
            {
                m_normTypeCombo->blockSignals(true);
                m_normTypeCombo->setCurrentIndex(i);
                m_normTypeCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue crossCheckJson = model["crossCheck"];
    if (!crossCheckJson.isUndefined())
    {
        m_crossCheck = crossCheckJson.toBool();
        m_crossCheckCheck->setChecked(m_crossCheck);
    }

    QJsonValue maxRatioJson = model["maxRatio"];
    if (!maxRatioJson.isUndefined())
    {
        m_maxRatio = maxRatioJson.toDouble();
        m_maxRatioSpin->setValue(m_maxRatio);
    }

    QJsonValue maxDistanceJson = model["maxDistance"];
    if (!maxDistanceJson.isUndefined())
    {
        m_maxDistance = maxDistanceJson.toDouble();
        m_maxDistanceSpin->setValue(m_maxDistance);
    }

    QJsonValue maxMatchesJson = model["maxMatches"];
    if (!maxMatchesJson.isUndefined())
    {
        m_maxMatches = maxMatchesJson.toInt();
        m_maxMatchesSpin->setValue(m_maxMatches);
    }

    matchFeatures();
}

} // namespace VisionBox
