/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Pyramid Model Implementation
 ******************************************************************************/

#include "ImagePyramidModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ImagePyramidModel::ImagePyramidModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Pyramid type selector
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Type:"));
    m_pyramidTypeCombo = new QComboBox();
    m_pyramidTypeCombo->addItem("Gaussian Pyramid", static_cast<int>(PyramidType::Gaussian));
    m_pyramidTypeCombo->addItem("Laplacian Pyramid", static_cast<int>(PyramidType::Laplacian));
    typeLayout->addWidget(m_pyramidTypeCombo);
    layout->addLayout(typeLayout);

    // Number of levels
    auto* levelsLayout = new QHBoxLayout();
    levelsLayout->addWidget(new QLabel("Levels:"));
    m_levelsSpin = new QSpinBox();
    m_levelsSpin->setRange(1, 10);
    m_levelsSpin->setValue(4);
    levelsLayout->addWidget(m_levelsSpin);
    layout->addLayout(levelsLayout);

    // Display level selector
    auto* displayLayout = new QHBoxLayout();
    displayLayout->addWidget(new QLabel("Display Level:"));
    m_displayLevelSpin = new QSpinBox();
    m_displayLevelSpin->setRange(0, 9);
    m_displayLevelSpin->setValue(0);
    displayLayout->addWidget(m_displayLevelSpin);
    layout->addLayout(displayLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_pyramidTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImagePyramidModel::onPyramidTypeChanged);
    connect(m_levelsSpin, &QSpinBox::valueChanged,
            this, &ImagePyramidModel::onLevelsChanged);
    connect(m_displayLevelSpin, &QSpinBox::valueChanged,
            this, &ImagePyramidModel::onDisplayLevelChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ImagePyramidModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType ImagePyramidModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ImagePyramidModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void ImagePyramidModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                  QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    buildPyramid();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ImagePyramidModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ImagePyramidModel::onPyramidTypeChanged(int index)
{
    m_pyramidType = static_cast<PyramidType>(m_pyramidTypeCombo->itemData(index).toInt());
    buildPyramid();
}

void ImagePyramidModel::onLevelsChanged(int value)
{
    m_levels = value;
    // Update display level range
    m_displayLevelSpin->setRange(0, m_levels - 1);
    buildPyramid();
}

void ImagePyramidModel::onDisplayLevelChanged(int value)
{
    m_displayLevel = value;
    buildPyramid();
}

/*******************************************************************************
 * Pyramid Building
 ******************************************************************************/
void ImagePyramidModel::buildPyramid()
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
        // Clear previous pyramids
        m_gaussianPyramid.clear();
        m_laplacianPyramid.clear();

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

        if (m_pyramidType == PyramidType::Gaussian)
        {
            // Build Gaussian pyramid
            cv::Mat currentLevel = gray.clone();
            m_gaussianPyramid.push_back(currentLevel);

            for (int i = 1; i < m_levels; ++i)
            {
                cv::Mat downsampled;
                cv::pyrDown(currentLevel, downsampled);
                m_gaussianPyramid.push_back(downsampled);
                currentLevel = downsampled;
            }

            // Display selected level
            if (m_displayLevel < static_cast<int>(m_gaussianPyramid.size()))
            {
                cv::Mat level = m_gaussianPyramid[m_displayLevel];
                cv::Mat output;
                cv::cvtColor(level, output, cv::COLOR_GRAY2BGR);
                m_outputImage = std::make_shared<ImageData>(output);
            }
        }
        else  // Laplacian pyramid
        {
            // Build Gaussian pyramid first
            cv::Mat currentLevel = gray.clone();
            m_gaussianPyramid.push_back(currentLevel);

            for (int i = 1; i < m_levels; ++i)
            {
                cv::Mat downsampled;
                cv::pyrDown(currentLevel, downsampled);
                m_gaussianPyramid.push_back(downsampled);
                currentLevel = downsampled;
            }

            // Build Laplacian pyramid
            for (size_t i = 0; i < m_gaussianPyramid.size() - 1; ++i)
            {
                cv::Mat upsampled;
                cv::pyrUp(m_gaussianPyramid[i + 1], upsampled,
                         cv::Size(m_gaussianPyramid[i].cols * 2,
                                 m_gaussianPyramid[i].rows * 2));

                // Ensure sizes match
                if (upsampled.size() != m_gaussianPyramid[i].size())
                {
                    cv::resize(upsampled, upsampled, m_gaussianPyramid[i].size());
                }

                cv::Mat laplacian;
                cv::absdiff(m_gaussianPyramid[i], upsampled, laplacian);
                m_laplacianPyramid.push_back(laplacian);
            }

            // Add the last level of Gaussian pyramid
            m_laplacianPyramid.push_back(m_gaussianPyramid.back());

            // Display selected level
            if (m_displayLevel < static_cast<int>(m_laplacianPyramid.size()))
            {
                cv::Mat level = m_laplacianPyramid[m_displayLevel];
                cv::Mat output;
                cv::cvtColor(level, output, cv::COLOR_GRAY2BGR);
                m_outputImage = std::make_shared<ImageData>(output);
            }
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
QJsonObject ImagePyramidModel::save() const
{
    QJsonObject modelJson;
    modelJson["pyramidType"] = static_cast<int>(m_pyramidType);
    modelJson["levels"] = m_levels;
    modelJson["displayLevel"] = m_displayLevel;
    return modelJson;
}

void ImagePyramidModel::load(QJsonObject const& model)
{
    QJsonValue typeJson = model["pyramidType"];
    if (!typeJson.isUndefined())
    {
        m_pyramidType = static_cast<PyramidType>(typeJson.toInt());
        m_pyramidTypeCombo->setCurrentIndex(static_cast<int>(m_pyramidType));
    }

    QJsonValue levelsJson = model["levels"];
    if (!levelsJson.isUndefined())
    {
        m_levels = levelsJson.toInt();
        m_levelsSpin->setValue(m_levels);
    }

    QJsonValue displayJson = model["displayLevel"];
    if (!displayJson.isUndefined())
    {
        m_displayLevel = displayJson.toInt();
        m_displayLevelSpin->setValue(m_displayLevel);
    }

    buildPyramid();
}

} // namespace VisionBox
