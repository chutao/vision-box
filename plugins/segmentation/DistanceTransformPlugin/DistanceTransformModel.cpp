/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Distance Transform Model Implementation
 ******************************************************************************/

#include "DistanceTransformModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
DistanceTransformModel::DistanceTransformModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Distance type selector
    auto* distLayout = new QHBoxLayout();
    distLayout->addWidget(new QLabel("Distance Type:"));
    m_distanceTypeCombo = new QComboBox();
    m_distanceTypeCombo->addItem("L1 (Manhattan)", static_cast<int>(DistanceType::L1));
    m_distanceTypeCombo->addItem("L2 (Euclidean)", static_cast<int>(DistanceType::L2));
    m_distanceTypeCombo->addItem("C (Checkerboard)", static_cast<int>(DistanceType::C));
    distLayout->addWidget(m_distanceTypeCombo);
    layout->addLayout(distLayout);

    // Label type selector
    auto* labelLayout = new QHBoxLayout();
    labelLayout->addWidget(new QLabel("Output:"));
    m_labelTypeCombo = new QComboBox();
    m_labelTypeCombo->addItem("Distance Map", static_cast<int>(LabelType::Distance));
    m_labelTypeCombo->addItem("Component Labels", static_cast<int>(LabelType::Labels));
    labelLayout->addWidget(m_labelTypeCombo);
    layout->addLayout(labelLayout);

    // Normalize checkbox
    m_normalizeCheck = new QCheckBox("Normalize Output");
    m_normalizeCheck->setChecked(true);
    layout->addWidget(m_normalizeCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_distanceTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DistanceTransformModel::onDistanceTypeChanged);
    connect(m_labelTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DistanceTransformModel::onLabelTypeChanged);
    connect(m_normalizeCheck, &QCheckBox::stateChanged,
            this, &DistanceTransformModel::onNormalizeChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int DistanceTransformModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType DistanceTransformModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> DistanceTransformModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void DistanceTransformModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                      QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    computeDistanceTransform();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* DistanceTransformModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void DistanceTransformModel::onDistanceTypeChanged(int index)
{
    m_distanceType = static_cast<DistanceType>(m_distanceTypeCombo->itemData(index).toInt());
    computeDistanceTransform();
}

void DistanceTransformModel::onLabelTypeChanged(int index)
{
    m_labelType = static_cast<LabelType>(m_labelTypeCombo->itemData(index).toInt());
    computeDistanceTransform();
}

void DistanceTransformModel::onNormalizeChanged(int state)
{
    m_normalize = (state == Qt::Checked);
    computeDistanceTransform();
}

/*******************************************************************************
 * Distance Transform Computation
 ******************************************************************************/
void DistanceTransformModel::computeDistanceTransform()
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

        // Threshold to binary image
        cv::Mat binary;
        cv::threshold(gray, binary, 128, 255, cv::THRESH_BINARY);

        // Invert if needed (distance transform works on foreground = 255)
        // We want white (255) to be foreground
        cv::Mat binaryInv;
        cv::bitwise_not(binary, binaryInv);

        // Compute distance transform
        cv::Mat dist;
        int distanceType = cv::DIST_L2;
        int maskSize = 3;  // 3x3 mask for L2, 5x5 for L1/C

        switch (m_distanceType)
        {
            case DistanceType::L1:
                distanceType = cv::DIST_L1;
                maskSize = 3;
                break;
            case DistanceType::L2:
                distanceType = cv::DIST_L2;
                maskSize = 5;
                break;
            case DistanceType::C:
                distanceType = cv::DIST_C;
                maskSize = 3;
                break;
        }

        if (m_labelType == LabelType::Labels)
        {
            // Compute distance transform with labels
            cv::Mat labels;
            cv::distanceTransform(binaryInv, dist, labels, distanceType, maskSize,
                                 cv::DIST_LABEL_PIXEL);

            // Visualize labels with colors
            cv::Mat output = cv::Mat::zeros(binaryInv.size(), CV_8UC3);
            int maxLabel = *std::max_element(labels.begin<int>(), labels.end<int>());

            for (int i = 1; i <= maxLabel; ++i)
            {
                cv::Mat mask = (labels == i);
                cv::Vec3b color = cv::Vec3b(rand() % 256, rand() % 256, rand() % 256);
                output.setTo(color, mask);
            }

            m_outputImage = std::make_shared<ImageData>(output);
        }
        else
        {
            // Compute distance transform
            cv::distanceTransform(binaryInv, dist, distanceType, maskSize);

            // Normalize for visualization
            cv::Mat output;
            if (m_normalize)
            {
                cv::normalize(dist, output, 0, 255, cv::NORM_MINMAX);
            }
            else
            {
                dist.convertTo(output, CV_8U);
            }

            // Convert to BGR for visualization
            cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);

            // Apply colormap
            cv::applyColorMap(output, output, cv::COLORMAP_JET);

            m_outputImage = std::make_shared<ImageData>(output);
        }

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
QJsonObject DistanceTransformModel::save() const
{
    QJsonObject modelJson;
    modelJson["distanceType"] = static_cast<int>(m_distanceType);
    modelJson["labelType"] = static_cast<int>(m_labelType);
    modelJson["normalize"] = m_normalize;
    return modelJson;
}

void DistanceTransformModel::load(QJsonObject const& model)
{
    QJsonValue distJson = model["distanceType"];
    if (!distJson.isUndefined())
    {
        m_distanceType = static_cast<DistanceType>(distJson.toInt());
        m_distanceTypeCombo->setCurrentIndex(static_cast<int>(m_distanceType));
    }

    QJsonValue labelJson = model["labelType"];
    if (!labelJson.isUndefined())
    {
        m_labelType = static_cast<LabelType>(labelJson.toInt());
        m_labelTypeCombo->setCurrentIndex(static_cast<int>(m_labelType));
    }

    QJsonValue normJson = model["normalize"];
    if (!normJson.isUndefined())
    {
        m_normalize = normJson.toBool();
        m_normalizeCheck->setChecked(m_normalize);
    }

    computeDistanceTransform();
}

} // namespace VisionBox
