/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * K-Means Segmentation Model Implementation
 ******************************************************************************/

#include "KMeansSegmentationModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
KMeansSegmentationModel::KMeansSegmentationModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // K (number of clusters) selector
    auto* kLayout = new QHBoxLayout();
    kLayout->addWidget(new QLabel("Clusters (K):"));
    m_kSpin = new QSpinBox();
    m_kSpin->setRange(2, 16);
    m_kSpin->setValue(3);
    kLayout->addWidget(m_kSpin);
    layout->addLayout(kLayout);

    // Attempts selector
    auto* attemptsLayout = new QHBoxLayout();
    attemptsLayout->addWidget(new QLabel("Attempts:"));
    m_attemptsCombo = new QComboBox();
    m_attemptsCombo->addItem("Low (3)", Low);
    m_attemptsCombo->addItem("Medium (10)", Medium);
    m_attemptsCombo->addItem("High (20)", High);
    m_attemptsCombo->setCurrentIndex(static_cast<int>(Medium));
    attemptsLayout->addWidget(m_attemptsCombo);
    layout->addLayout(attemptsLayout);

    // Random centers checkbox
    m_randomCentersCheck = new QCheckBox("Use Random Centers");
    layout->addWidget(m_randomCentersCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_kSpin, &QSpinBox::valueChanged,
            this, &KMeansSegmentationModel::onKChanged);
    connect(m_attemptsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KMeansSegmentationModel::onAttemptsChanged);
    connect(m_randomCentersCheck, &QCheckBox::stateChanged,
            this, &KMeansSegmentationModel::onRandomCentersChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int KMeansSegmentationModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType KMeansSegmentationModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> KMeansSegmentationModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void KMeansSegmentationModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                       QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applySegmentation();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* KMeansSegmentationModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void KMeansSegmentationModel::applySegmentation()
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
        // Convert to float for K-means
        cv::Mat data;
        input.convertTo(data, CV_32F);

        // Reshape to be a 1D array of pixels (with 3 channels for BGR)
        data = data.reshape(1, data.total());

        // Convert to std::vector for k-means
        std::vector<float> samples;
        data.copyTo(samples);

        // Number of attempts based on selection
        int attempts;
        switch (m_attempts)
        {
            case Low:
                attempts = 3;
                break;
            case Medium:
                attempts = 10;
                break;
            case High:
                attempts = 20;
                break;
            default:
                attempts = 10;
                break;
        }

        // Run K-means clustering
        cv::Mat labels, centers;
        cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 100, 0.01);

        double compactness = cv::kmeans(
            samples,                                                      // Data
            m_k,                                                          // Number of clusters
            labels,                                                      // Output cluster labels
            criteria,                                                     // Termination criteria
            attempts,                                                     // Number of attempts
            m_useRandomCenters ? cv::KMEANS_RANDOM_CENTERS : cv::KMEANS_PP_CENTERS,  // Flags
            centers                                                      // Output cluster centers
        );

        // Quantize the image based on cluster centers
        cv::Mat output(input.size(), input.type());
        cv::Mat centersU8;
        centers.convertTo(centersU8, CV_8U);

        cv::MatIterator_<cv::Vec3b> it = output.begin<cv::Vec3b>();
        cv::MatConstIterator_<cv::Vec3b> inputIt = input.begin<cv::Vec3b>();
        cv::MatConstIterator_<int> labelIt = labels.begin<int>();

        for (size_t i = 0; i < input.total(); ++i, ++it, ++inputIt, ++labelIt)
        {
            // Find the nearest cluster center for each pixel
            int clusterIdx = *labelIt;
            cv::Vec3b centerColor(
                centersU8.at<uint8_t>(clusterIdx, 2),  // B
                centersU8.at<uint8_t>(clusterIdx, 1),  // G
                centersU8.at<uint8_t>(clusterIdx, 0)   // R
            );
            *it = centerColor;
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

void KMeansSegmentationModel::onKChanged(int value)
{
    m_k = value;
    applySegmentation();
}

void KMeansSegmentationModel::onAttemptsChanged(int index)
{
    m_attempts = static_cast<AttemptLevel>(m_attemptsCombo->itemData(index).toInt());
    applySegmentation();
}

void KMeansSegmentationModel::onRandomCentersChanged(int state)
{
    m_useRandomCenters = (state == Qt::Checked);
    applySegmentation();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject KMeansSegmentationModel::save() const
{
    QJsonObject modelJson;
    modelJson["k"] = m_k;
    modelJson["attempts"] = static_cast<int>(m_attempts);
    modelJson["useRandomCenters"] = m_useRandomCenters;
    return modelJson;
}

void KMeansSegmentationModel::load(QJsonObject const& model)
{
    QJsonValue kJson = model["k"];
    if (!kJson.isUndefined())
    {
        m_k = kJson.toInt();
        m_kSpin->setValue(m_k);
    }

    QJsonValue attemptsJson = model["attempts"];
    if (!attemptsJson.isUndefined())
    {
        m_attempts = static_cast<AttemptLevel>(attemptsJson.toInt());
        m_attemptsCombo->setCurrentIndex(static_cast<int>(m_attempts));
    }

    QJsonValue randomJson = model["useRandomCenters"];
    if (!randomJson.isUndefined())
    {
        m_useRandomCenters = randomJson.toBool();
        m_randomCentersCheck->setChecked(m_useRandomCenters);
    }

    applySegmentation();
}

} // namespace VisionBox
