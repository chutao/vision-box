/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Contour Finder Model Implementation
 ******************************************************************************/

#include "ContourFinderModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ContourFinderModel::ContourFinderModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Retrieval mode selector
    auto* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Mode:"));
    m_modeCombo = new QComboBox();
    m_modeCombo->addItem("External", External);
    m_modeCombo->addItem("List", List);
    m_modeCombo->addItem("Connected Components", CComp);
    m_modeCombo->addItem("Hierarchy Tree", Tree);
    m_modeCombo->addItem("Flood Fill", FloodFill);
    modeLayout->addWidget(m_modeCombo);
    layout->addLayout(modeLayout);

    // Approximation method selector
    auto* approxLayout = new QHBoxLayout();
    approxLayout->addWidget(new QLabel("Approximation:"));
    m_approxCombo = new QComboBox();
    m_approxCombo->addItem("None", None);
    m_approxCombo->addItem("Simple", Simple);
    m_approxCombo->addItem("TC89 L1", TC89_L1);
    m_approxCombo->addItem("TC89 KCOS", TC89_KCOS);
    approxLayout->addWidget(m_approxCombo);
    layout->addLayout(approxLayout);

    // Min area filter
    auto* minAreaLayout = new QHBoxLayout();
    minAreaLayout->addWidget(new QLabel("Min Area:"));
    m_minAreaSpin = new QSpinBox();
    m_minAreaSpin->setRange(0, 100000);
    m_minAreaSpin->setValue(0);
    minAreaLayout->addWidget(m_minAreaSpin);
    layout->addLayout(minAreaLayout);

    // Max area filter
    auto* maxAreaLayout = new QHBoxLayout();
    maxAreaLayout->addWidget(new QLabel("Max Area:"));
    m_maxAreaSpin = new QSpinBox();
    m_maxAreaSpin->setRange(0, 100000);
    m_maxAreaSpin->setValue(100000);
    maxAreaLayout->addWidget(m_maxAreaSpin);
    layout->addLayout(maxAreaLayout);

    // Draw contours checkbox
    m_drawContoursCheck = new QCheckBox("Draw Contours");
    m_drawContoursCheck->setChecked(true);
    layout->addWidget(m_drawContoursCheck);

    // Line thickness
    auto* thicknessLayout = new QHBoxLayout();
    thicknessLayout->addWidget(new QLabel("Thickness:"));
    m_thicknessSpin = new QSpinBox();
    m_thicknessSpin->setRange(1, 10);
    m_thicknessSpin->setValue(1);
    thicknessLayout->addWidget(m_thicknessSpin);
    layout->addLayout(thicknessLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ContourFinderModel::onModeChanged);
    connect(m_approxCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ContourFinderModel::onApproxChanged);
    connect(m_minAreaSpin, &QSpinBox::valueChanged,
            this, &ContourFinderModel::onMinAreaChanged);
    connect(m_maxAreaSpin, &QSpinBox::valueChanged,
            this, &ContourFinderModel::onMaxAreaChanged);
    connect(m_drawContoursCheck, &QCheckBox::stateChanged,
            this, &ContourFinderModel::onDrawContoursChanged);
    connect(m_thicknessSpin, &QSpinBox::valueChanged,
            this, &ContourFinderModel::onThicknessChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ContourFinderModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ContourFinderModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ContourFinderModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ContourFinderModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                 QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    findContours();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ContourFinderModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ContourFinderModel::findContours()
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
        // Convert to grayscale
        cv::Mat gray;
        if (input.channels() > 1)
        {
            cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        }
        else
        {
            gray = input;
        }

        // Apply threshold to get binary image
        cv::Mat thresh;
        cv::threshold(gray, thresh, 127, 255, cv::THRESH_BINARY);

        // Find contours
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;

        int mode;
        switch (m_retrievalMode)
        {
            case External:
                mode = cv::RETR_EXTERNAL;
                break;
            case List:
                mode = cv::RETR_LIST;
                break;
            case CComp:
                mode = cv::RETR_CCOMP;
                break;
            case Tree:
                mode = cv::RETR_TREE;
                break;
            case FloodFill:
                mode = cv::RETR_FLOODFILL;
                break;
            default:
                mode = cv::RETR_EXTERNAL;
                break;
        }

        int approxMethod;
        switch (m_approximation)
        {
            case None:
                approxMethod = cv::CHAIN_APPROX_NONE;
                break;
            case Simple:
                approxMethod = cv::CHAIN_APPROX_SIMPLE;
                break;
            case TC89_L1:
                approxMethod = cv::CHAIN_APPROX_TC89_L1;
                break;
            case TC89_KCOS:
                approxMethod = cv::CHAIN_APPROX_TC89_KCOS;
                break;
            default:
                approxMethod = cv::CHAIN_APPROX_SIMPLE;
                break;
        }

        cv::findContours(thresh, contours, hierarchy, mode, approxMethod);

        // Filter contours by area
        std::vector<std::vector<cv::Point>> filteredContours;
        for (const auto& contour : contours)
        {
            double area = cv::contourArea(contour);
            if (area >= m_minArea && area <= m_maxArea)
            {
                filteredContours.push_back(contour);
            }
        }

        // Draw contours
        cv::Mat output;
        if (m_drawContours)
        {
            // Create color image for drawing
            if (input.channels() == 1)
            {
                cv::cvtColor(input, output, cv::COLOR_GRAY2BGR);
            }
            else
            {
                output = input.clone();
            }

            // Draw contours in green
            cv::Scalar color(0, 255, 0);  // Green in BGR
            for (size_t i = 0; i < filteredContours.size(); i++)
            {
                cv::drawContours(output, filteredContours, static_cast<int>(i), color,
                                m_thickness);
            }
        }
        else
        {
            // Just return the thresholded image
            cv::cvtColor(thresh, output, cv::COLOR_GRAY2BGR);
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

void ContourFinderModel::onModeChanged(int index)
{
    m_retrievalMode = static_cast<RetrievalMode>(m_modeCombo->itemData(index).toInt());
    findContours();
}

void ContourFinderModel::onApproxChanged(int index)
{
    m_approximation = static_cast<ContourApproximation>(m_approxCombo->itemData(index).toInt());
    findContours();
}

void ContourFinderModel::onMinAreaChanged(int value)
{
    m_minArea = value;
    findContours();
}

void ContourFinderModel::onMaxAreaChanged(int value)
{
    m_maxArea = value;
    findContours();
}

void ContourFinderModel::onDrawContoursChanged(int state)
{
    m_drawContours = (state == Qt::Checked);
    findContours();
}

void ContourFinderModel::onThicknessChanged(int value)
{
    m_thickness = value;
    findContours();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ContourFinderModel::save() const
{
    QJsonObject modelJson;
    modelJson["retrievalMode"] = static_cast<int>(m_retrievalMode);
    modelJson["approximation"] = static_cast<int>(m_approximation);
    modelJson["minArea"] = m_minArea;
    modelJson["maxArea"] = m_maxArea;
    modelJson["drawContours"] = m_drawContours;
    modelJson["thickness"] = m_thickness;
    return modelJson;
}

void ContourFinderModel::restore(QJsonObject const& model)
{
    QJsonValue modeJson = model["retrievalMode"];
    if (!modeJson.isUndefined())
    {
        m_retrievalMode = static_cast<RetrievalMode>(modeJson.toInt());
        m_modeCombo->setCurrentIndex(static_cast<int>(m_retrievalMode));
    }

    QJsonValue approxJson = model["approximation"];
    if (!approxJson.isUndefined())
    {
        m_approximation = static_cast<ContourApproximation>(approxJson.toInt());
        m_approxCombo->setCurrentIndex(static_cast<int>(m_approximation));
    }

    QJsonValue minAreaJson = model["minArea"];
    if (!minAreaJson.isUndefined())
    {
        m_minArea = minAreaJson.toInt();
        m_minAreaSpin->setValue(m_minArea);
    }

    QJsonValue maxAreaJson = model["maxArea"];
    if (!maxAreaJson.isUndefined())
    {
        m_maxArea = maxAreaJson.toInt();
        m_maxAreaSpin->setValue(m_maxArea);
    }

    QJsonValue drawJson = model["drawContours"];
    if (!drawJson.isUndefined())
    {
        m_drawContours = drawJson.toBool();
        m_drawContoursCheck->setChecked(m_drawContours);
    }

    QJsonValue thicknessJson = model["thickness"];
    if (!thicknessJson.isUndefined())
    {
        m_thickness = thicknessJson.toInt();
        m_thicknessSpin->setValue(m_thickness);
    }

    findContours();
}

} // namespace VisionBox
