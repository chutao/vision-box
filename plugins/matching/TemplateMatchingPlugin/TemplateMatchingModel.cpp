/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Template Matching Model Implementation
 ******************************************************************************/

#include "TemplateMatchingModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
TemplateMatchingModel::TemplateMatchingModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Method selector
    auto* methodLayout = new QHBoxLayout();
    methodLayout->addWidget(new QLabel("Method:"));
    m_methodCombo = new QComboBox();
    m_methodCombo->addItem("Squared Difference", SquaredDifference);
    m_methodCombo->addItem("Normalized Cross-Corr", NormalizedCrossCorrelation);
    m_methodCombo->addItem("Correlation Coeff", CorrelationCoefficient);
    m_methodCombo->addItem("Normalized Sq Diff", NormalizedSquaredDifference);
    methodLayout->addWidget(m_methodCombo);
    layout->addLayout(methodLayout);

    // Threshold selector
    auto* thresholdLayout = new QHBoxLayout();
    thresholdLayout->addWidget(new QLabel("Threshold:"));
    m_thresholdSpin = new QDoubleSpinBox();
    m_thresholdSpin->setRange(0.0, 1.0);
    m_thresholdSpin->setSingleStep(0.05);
    m_thresholdSpin->setDecimals(2);
    m_thresholdSpin->setValue(0.8);
    thresholdLayout->addWidget(m_thresholdSpin);
    layout->addLayout(thresholdLayout);

    // Draw bounding box checkbox
    m_drawBoxCheck = new QCheckBox("Draw Bounding Box");
    m_drawBoxCheck->setChecked(true);
    layout->addWidget(m_drawBoxCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TemplateMatchingModel::onMethodChanged);
    connect(m_thresholdSpin, &QDoubleSpinBox::valueChanged,
            this, &TemplateMatchingModel::onThresholdChanged);
    connect(m_drawBoxCheck, &QCheckBox::stateChanged,
            this, &TemplateMatchingModel::onDrawBoxChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int TemplateMatchingModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 2;  // Image (port 0) and Template (port 1)
    }
    else
    {
        return 1;
    }
}

QtNodes::NodeDataType TemplateMatchingModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> TemplateMatchingModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void TemplateMatchingModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                QtNodes::PortIndex portIndex)
{
    if (portIndex == 0)
    {
        m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    }
    else if (portIndex == 1)
    {
        m_templateImage = std::dynamic_pointer_cast<ImageData>(data);
    }

    applyMatching();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* TemplateMatchingModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void TemplateMatchingModel::applyMatching()
{
    if (!m_inputImage || !m_templateImage)
    {
        m_outputImage = nullptr;
        return;
    }

    cv::Mat input = m_inputImage->image();
    cv::Mat templ = m_templateImage->image();

    if (input.empty() || templ.empty())
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
        return;
    }

    try
    {
        // Convert to grayscale if needed
        cv::Mat inputGray, templGray;
        if (input.channels() > 1)
        {
            cv::cvtColor(input, inputGray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            inputGray = input;
        }

        if (templ.channels() > 1)
        {
            cv::cvtColor(templ, templGray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            templGray = templ;
        }

        // Ensure template is smaller than image
        if (templGray.rows > inputGray.rows || templGray.cols > inputGray.cols)
        {
            // Template is larger than image, can't match
            m_outputImage = std::make_shared<ImageData>(input.clone());
            Q_EMIT dataUpdated(0);
            return;
        }

        // Map method to OpenCV enum
        int method;
        switch (m_method)
        {
            case SquaredDifference:
                method = cv::TM_SQDIFF;
                break;
            case NormalizedCrossCorrelation:
                method = cv::TM_CCORR_NORMED;
                break;
            case CorrelationCoefficient:
                method = cv::TM_CCOEFF_NORMED;
                break;
            case NormalizedSquaredDifference:
                method = cv::TM_SQDIFF_NORMED;
                break;
            default:
                method = cv::TM_CCORR_NORMED;
                break;
        }

        // Perform template matching
        cv::Mat result;
        cv::matchTemplate(inputGray, templGray, result, method);

        // Normalize result for better visualization
        cv::normalize(result, result, 0, 255, cv::NORM_MINMAX);

        // Find best match location
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

        // For SQDIFF methods, minimum is best match
        // For CORR methods, maximum is best match
        cv::Point matchLoc;
        if (method == cv::TM_SQDIFF || method == cv::TM_SQDIFF_NORMED)
        {
            matchLoc = minLoc;
        }
        else
        {
            matchLoc = maxLoc;
        }

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

        // Draw bounding box around matched region
        if (m_drawBox)
        {
            cv::rectangle(output, matchLoc,
                         cv::Point(matchLoc.x + templGray.cols, matchLoc.y + templGray.rows),
                         cv::Scalar(0, 255, 0), 2);  // Green in BGR
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

void TemplateMatchingModel::onMethodChanged(int index)
{
    m_method = static_cast<MatchingMethod>(m_methodCombo->itemData(index).toInt());
    applyMatching();
}

void TemplateMatchingModel::onThresholdChanged(double value)
{
    m_threshold = value;
    applyMatching();
}

void TemplateMatchingModel::onDrawBoxChanged(int state)
{
    m_drawBox = (state == Qt::Checked);
    applyMatching();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject TemplateMatchingModel::save() const
{
    QJsonObject modelJson;
    modelJson["method"] = static_cast<int>(m_method);
    modelJson["threshold"] = m_threshold;
    modelJson["drawBox"] = m_drawBox;
    return modelJson;
}

void TemplateMatchingModel::load(QJsonObject const& model)
{
    QJsonValue methodJson = model["method"];
    if (!methodJson.isUndefined())
    {
        m_method = static_cast<MatchingMethod>(methodJson.toInt());
        m_methodCombo->setCurrentIndex(static_cast<int>(m_method));
    }

    QJsonValue thresholdJson = model["threshold"];
    if (!thresholdJson.isUndefined())
    {
        m_threshold = thresholdJson.toDouble();
        m_thresholdSpin->setValue(m_threshold);
    }

    QJsonValue drawJson = model["drawBox"];
    if (!drawJson.isUndefined())
    {
        m_drawBox = drawJson.toBool();
        m_drawBoxCheck->setChecked(m_drawBox);
    }

    applyMatching();
}

} // namespace VisionBox
