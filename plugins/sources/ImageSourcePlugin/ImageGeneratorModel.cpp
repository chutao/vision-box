/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Generator Node Model Implementation
 ******************************************************************************/

#include "ImageGeneratorModel.h"
#include "core/VisionDataTypes.h"
#include "core/PerformanceMonitor.h"
#include <opencv2/opencv.hpp>
#include <QRandomGenerator>
#include <QTimer>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
ImageGeneratorModel::ImageGeneratorModel()
    : m_imageData(nullptr)
    , m_generateTimer(new QTimer(this))
{
    // Setup debounce timer (50ms delay to prevent excessive updates)
    m_generateTimer->setSingleShot(true);
    m_generateTimer->setInterval(50);  // 50ms debounce delay
    connect(m_generateTimer, &QTimer::timeout,
            this, &ImageGeneratorModel::performGenerate);

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Pattern selection
    auto* patternLayout = new QHBoxLayout();
    patternLayout->addWidget(new QLabel("Pattern:"));
    m_patternCombo = new QComboBox();
    m_patternCombo->addItem("Solid Color", SolidColor);
    m_patternCombo->addItem("Gradient (Horizontal)", GradientHorizontal);
    m_patternCombo->addItem("Gradient (Vertical)", GradientVertical);
    m_patternCombo->addItem("Gradient (Diagonal)", GradientDiagonal);
    m_patternCombo->addItem("Checkerboard", Checkerboard);
    m_patternCombo->addItem("Grid", Grid);
    m_patternCombo->addItem("Circles", Circles);
    m_patternCombo->addItem("Rectangles", Rectangles);
    m_patternCombo->addItem("Lines", Lines);
    m_patternCombo->addItem("Gaussian Noise", GaussianNoise);
    m_patternCombo->addItem("Uniform Noise", UniformNoise);
    m_patternCombo->setMinimumWidth(150);
    patternLayout->addWidget(m_patternCombo);
    layout->addLayout(patternLayout);

    // Dimensions
    auto* dimLayout = new QHBoxLayout();
    dimLayout->addWidget(new QLabel("Width:"));
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(64, 4096);
    m_widthSpin->setValue(640);
    m_widthSpin->setMinimumWidth(80);
    dimLayout->addWidget(m_widthSpin);

    dimLayout->addWidget(new QLabel("Height:"));
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(64, 4096);
    m_heightSpin->setValue(480);
    m_heightSpin->setMinimumWidth(80);
    dimLayout->addWidget(m_heightSpin);
    layout->addLayout(dimLayout);

    // Channels
    auto* channelLayout = new QHBoxLayout();
    channelLayout->addWidget(new QLabel("Channels:"));
    m_channelsCombo = new QComboBox();
    m_channelsCombo->addItem("Grayscale (1)", 1);
    m_channelsCombo->addItem("BGR (3)", 3);
    m_channelsCombo->addItem("RGBA (4)", 4);
    m_channelsCombo->setCurrentIndex(1);
    m_channelsCombo->setMinimumWidth(120);
    channelLayout->addWidget(m_channelsCombo);
    layout->addLayout(channelLayout);

    // Value 1
    m_value1Label = new QLabel("Intensity:");
    auto* value1Layout = new QHBoxLayout();
    value1Layout->addWidget(m_value1Label);
    m_value1Spin = new QDoubleSpinBox();
    m_value1Spin->setRange(0, 255);
    m_value1Spin->setValue(128);
    m_value1Spin->setSingleStep(1);
    m_value1Spin->setMinimumWidth(100);
    value1Layout->addWidget(m_value1Spin);
    layout->addLayout(value1Layout);

    // Value 2
    m_value2Label = new QLabel("Size/Variance:");
    auto* value2Layout = new QHBoxLayout();
    value2Layout->addWidget(m_value2Label);
    m_value2Spin = new QDoubleSpinBox();
    m_value2Spin->setRange(1, 1000);
    m_value2Spin->setValue(30);
    m_value2Spin->setSingleStep(1);
    m_value2Spin->setMinimumWidth(100);
    value2Layout->addWidget(m_value2Spin);
    layout->addLayout(value2Layout);

    // Random colors
    m_randomCheck = new QCheckBox("Random Colors");
    layout->addWidget(m_randomCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_patternCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageGeneratorModel::onPatternChanged);
    connect(m_widthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ImageGeneratorModel::onParamChanged);
    connect(m_heightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ImageGeneratorModel::onParamChanged);
    connect(m_channelsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageGeneratorModel::onParamChanged);
    connect(m_value1Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageGeneratorModel::onParamChanged);
    connect(m_value2Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageGeneratorModel::onParamChanged);
    connect(m_randomCheck, &QCheckBox::stateChanged,
            this, &ImageGeneratorModel::onParamChanged);

    // Generate initial image
    onPatternChanged();
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ImageGeneratorModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 0; // No input ports
    }
    else
    {
        return 1; // One output port
    }
}

QtNodes::NodeDataType ImageGeneratorModel::dataType(
    QtNodes::PortType portType,
    QtNodes::PortIndex portIndex) const
{
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    return ImageData().type(); // "opencv_image"
}

/*******************************************************************************
 * Data Flow
 ******************************************************************************/
std::shared_ptr<QtNodes::NodeData> ImageGeneratorModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_imageData;
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ImageGeneratorModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ImageGeneratorModel::onPatternChanged()
{
    m_pattern = static_cast<PatternType>(m_patternCombo->currentData().toInt());
    m_channels = m_channelsCombo->currentData().toInt();

    // Update labels based on pattern
    switch (m_pattern)
    {
        case SolidColor:
            m_value1Label->setText("Intensity (0-255):");
            m_value1Spin->setRange(0, 255);
            m_value1Spin->setValue(128);
            m_value2Label->setText("Size:");
            m_value2Spin->setEnabled(false);
            m_randomCheck->setEnabled(true);
            break;

        case GradientHorizontal:
        case GradientVertical:
        case GradientDiagonal:
            m_value1Label->setText("Start Intensity:");
            m_value1Spin->setRange(0, 255);
            m_value1Spin->setValue(0);
            m_value2Label->setText("End Intensity:");
            m_value2Spin->setEnabled(true);
            m_value2Spin->setRange(0, 255);
            m_value2Spin->setValue(255);
            m_randomCheck->setEnabled(false);
            break;

        case Checkerboard:
            m_value1Label->setText("Square Size:");
            m_value1Spin->setRange(2, 200);
            m_value1Spin->setValue(32);
            m_value2Label->setText("Unused:");
            m_value2Spin->setEnabled(false);
            m_randomCheck->setEnabled(false);
            break;

        case Grid:
            m_value1Label->setText("Grid Size:");
            m_value1Spin->setRange(10, 200);
            m_value1Spin->setValue(64);
            m_value2Label->setText("Line Thickness:");
            m_value2Spin->setEnabled(true);
            m_value2Spin->setRange(1, 10);
            m_value2Spin->setValue(2);
            m_randomCheck->setEnabled(false);
            break;

        case Circles:
            m_value1Label->setText("Count:");
            m_value1Spin->setRange(1, 100);
            m_value1Spin->setValue(10);
            m_value2Label->setText("Radius:");
            m_value2Spin->setEnabled(true);
            m_value2Spin->setRange(5, 200);
            m_value2Spin->setValue(50);
            m_randomCheck->setEnabled(true);
            break;

        case Rectangles:
            m_value1Label->setText("Count:");
            m_value1Spin->setRange(1, 100);
            m_value1Spin->setValue(10);
            m_value2Label->setText("Max Size:");
            m_value2Spin->setEnabled(true);
            m_value2Spin->setRange(10, 500);
            m_value2Spin->setValue(100);
            m_randomCheck->setEnabled(true);
            break;

        case Lines:
            m_value1Label->setText("Count:");
            m_value1Spin->setRange(1, 100);
            m_value1Spin->setValue(20);
            m_value2Label->setText("Thickness:");
            m_value2Spin->setEnabled(true);
            m_value2Spin->setRange(1, 10);
            m_value2Spin->setValue(2);
            m_randomCheck->setEnabled(true);
            break;

        case GaussianNoise:
            m_value1Label->setText("Mean:");
            m_value1Spin->setRange(0, 255);
            m_value1Spin->setValue(128);
            m_value2Label->setText("Std Dev:");
            m_value2Spin->setEnabled(true);
            m_value2Spin->setRange(1, 100);
            m_value2Spin->setValue(30);
            m_randomCheck->setEnabled(false);
            break;

        case UniformNoise:
            m_value1Label->setText("Min:");
            m_value1Spin->setRange(0, 254);
            m_value1Spin->setValue(0);
            m_value2Label->setText("Max:");
            m_value2Spin->setEnabled(true);
            m_value2Spin->setRange(1, 255);
            m_value2Spin->setValue(255);
            m_randomCheck->setEnabled(false);
            break;
    }

    // Use debounce timer to prevent excessive updates during UI changes
    m_generateTimer->start();
}

void ImageGeneratorModel::onParamChanged()
{
    m_width = m_widthSpin->value();
    m_height = m_heightSpin->value();
    m_channels = m_channelsCombo->currentData().toInt();
    m_value1 = m_value1Spin->value();
    m_value2 = m_value2Spin->value();
    m_random = m_randomCheck->isChecked();

    // Use debounce timer to prevent excessive updates during parameter changes
    m_generateTimer->start();
}

/*******************************************************************************
 * Debounced Image Generation
 ******************************************************************************/
void ImageGeneratorModel::performGenerate()
{
    generateImage();
}

/*******************************************************************************
 * Image Generation
 ******************************************************************************/
void ImageGeneratorModel::generateImage()
{
    // Start performance timer
    PerformanceTimer timer(this, caption());

    cv::Mat image;

    switch (m_pattern)
    {
        case SolidColor:
            image = generateSolidColor();
            break;
        case GradientHorizontal:
        case GradientVertical:
        case GradientDiagonal:
            image = generateGradient();
            break;
        case Checkerboard:
            image = generateCheckerboard();
            break;
        case Grid:
            image = generateGrid();
            break;
        case Circles:
            image = generateCircles();
            break;
        case Rectangles:
            image = generateRectangles();
            break;
        case Lines:
            image = generateLines();
            break;
        case GaussianNoise:
        case UniformNoise:
            image = generateNoise();
            break;
    }

    m_imageData = std::make_shared<ImageData>(image);
    Q_EMIT dataUpdated(0);
}

cv::Mat ImageGeneratorModel::generateSolidColor()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type);

    if (m_random)
    {
        // Random color
        cv::Scalar color;
        if (m_channels == 1)
        {
            color = cv::Scalar(QRandomGenerator::global()->bounded(256));
        }
        else
        {
            color = cv::Scalar(QRandomGenerator::global()->bounded(256),
                              QRandomGenerator::global()->bounded(256),
                              QRandomGenerator::global()->bounded(256));
        }
        image.setTo(color);
    }
    else
    {
        cv::Scalar value = (m_channels == 1) ?
            cv::Scalar(m_value1) :
            cv::Scalar(m_value1, m_value1, m_value1);
        image.setTo(value);
    }

    return image;
}

cv::Mat ImageGeneratorModel::generateGradient()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type);

    double startVal = m_value1;
    double endVal = m_value2;

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            double t = 0.0;

            switch (m_pattern)
            {
                case GradientHorizontal:
                    t = static_cast<double>(x) / (m_width - 1);
                    break;
                case GradientVertical:
                    t = static_cast<double>(y) / (m_height - 1);
                    break;
                case GradientDiagonal:
                    t = (x + y) / (m_width + m_height - 2.0);
                    break;
                default:
                    t = 0.0;
                    break;
            }

            double value = startVal + t * (endVal - startVal);
            value = std::max(0.0, std::min(255.0, value));

            if (m_channels == 1)
            {
                image.at<uchar>(y, x) = static_cast<uchar>(value);
            }
            else
            {
                image.at<cv::Vec3b>(y, x) = cv::Vec3b::all(static_cast<uchar>(value));
            }
        }
    }

    return image;
}

cv::Mat ImageGeneratorModel::generateCheckerboard()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type);
    int squareSize = static_cast<int>(m_value1);

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            bool whiteSquare = ((x / squareSize) + (y / squareSize)) % 2 == 0;

            if (m_channels == 1)
            {
                image.at<uchar>(y, x) = whiteSquare ? 255 : 0;
            }
            else
            {
                image.at<cv::Vec3b>(y, x) = whiteSquare ?
                    cv::Vec3b(255, 255, 255) :
                    cv::Vec3b(0, 0, 0);
            }
        }
    }

    return image;
}

cv::Mat ImageGeneratorModel::generateGrid()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type, cv::Scalar::all(0));

    int gridSize = static_cast<int>(m_value1);
    int thickness = static_cast<int>(m_value2);
    cv::Scalar gridColor = (m_channels == 1) ?
        cv::Scalar(255) :
        cv::Scalar(255, 255, 255);

    // Draw vertical lines
    for (int x = gridSize; x < m_width; x += gridSize)
    {
        cv::line(image, cv::Point(x, 0), cv::Point(x, m_height - 1), gridColor, thickness);
    }

    // Draw horizontal lines
    for (int y = gridSize; y < m_height; y += gridSize)
    {
        cv::line(image, cv::Point(0, y), cv::Point(m_width - 1, y), gridColor, thickness);
    }

    return image;
}

cv::Mat ImageGeneratorModel::generateCircles()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type, cv::Scalar::all(0));

    int count = static_cast<int>(m_value1);
    int radius = static_cast<int>(m_value2);

    for (int i = 0; i < count; ++i)
    {
        cv::Point center(
            QRandomGenerator::global()->bounded(m_width),
            QRandomGenerator::global()->bounded(m_height)
        );

        cv::Scalar color;
        if (m_random)
        {
            if (m_channels == 1)
            {
                color = cv::Scalar(QRandomGenerator::global()->bounded(256));
            }
            else
            {
                color = cv::Scalar(QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256));
            }
        }
        else
        {
            color = (m_channels == 1) ?
                cv::Scalar(255) :
                cv::Scalar(255, 255, 255);
        }

        int actualRadius = radius;
        if (m_random)
        {
            actualRadius = QRandomGenerator::global()->bounded(10, radius + 1);
        }

        cv::circle(image, center, actualRadius, color, -1);
    }

    return image;
}

cv::Mat ImageGeneratorModel::generateRectangles()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type, cv::Scalar::all(0));

    int count = static_cast<int>(m_value1);
    int maxSize = static_cast<int>(m_value2);

    for (int i = 0; i < count; ++i)
    {
        int w = QRandomGenerator::global()->bounded(10, maxSize + 1);
        int h = QRandomGenerator::global()->bounded(10, maxSize + 1);
        int x = QRandomGenerator::global()->bounded(m_width - w);
        int y = QRandomGenerator::global()->bounded(m_height - h);

        cv::Scalar color;
        if (m_random)
        {
            if (m_channels == 1)
            {
                color = cv::Scalar(QRandomGenerator::global()->bounded(256));
            }
            else
            {
                color = cv::Scalar(QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256));
            }
        }
        else
        {
            color = (m_channels == 1) ?
                cv::Scalar(255) :
                cv::Scalar(255, 255, 255);
        }

        cv::rectangle(image, cv::Point(x, y), cv::Point(x + w, y + h), color, -1);
    }

    return image;
}

cv::Mat ImageGeneratorModel::generateLines()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type, cv::Scalar::all(0));

    int count = static_cast<int>(m_value1);
    int thickness = static_cast<int>(m_value2);

    for (int i = 0; i < count; ++i)
    {
        cv::Point pt1(QRandomGenerator::global()->bounded(m_width),
                     QRandomGenerator::global()->bounded(m_height));
        cv::Point pt2(QRandomGenerator::global()->bounded(m_width),
                     QRandomGenerator::global()->bounded(m_height));

        cv::Scalar color;
        if (m_random)
        {
            if (m_channels == 1)
            {
                color = cv::Scalar(QRandomGenerator::global()->bounded(256));
            }
            else
            {
                color = cv::Scalar(QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256));
            }
        }
        else
        {
            color = (m_channels == 1) ?
                cv::Scalar(255) :
                cv::Scalar(255, 255, 255);
        }

        cv::line(image, pt1, pt2, color, thickness);
    }

    return image;
}

cv::Mat ImageGeneratorModel::generateNoise()
{
    int type = (m_channels == 1) ? CV_8UC1 : CV_8UC3;
    cv::Mat image(m_height, m_width, type);

    if (m_pattern == GaussianNoise)
    {
        cv::Mat noise(m_height, m_width, type);
        double mean = m_value1;
        double stddev = m_value2;

        cv::randn(noise, cv::Scalar::all(mean), cv::Scalar::all(stddev));

        if (m_channels == 1)
        {
            for (int y = 0; y < m_height; ++y)
            {
                for (int x = 0; x < m_width; ++x)
                {
                    double val = noise.at<uchar>(y, x);
                    val = std::max(0.0, std::min(255.0, val));
                    image.at<uchar>(y, x) = static_cast<uchar>(val);
                }
            }
        }
        else
        {
            for (int y = 0; y < m_height; ++y)
            {
                for (int x = 0; x < m_width; ++x)
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        double val = noise.at<cv::Vec3b>(y, x)[c];
                        val = std::max(0.0, std::min(255.0, val));
                        image.at<cv::Vec3b>(y, x)[c] = static_cast<uchar>(val);
                    }
                }
            }
        }
    }
    else // Uniform noise
    {
        double minVal = m_value1;
        double maxVal = m_value2;

        cv::Mat noise(m_height, m_width, type);
        cv::randu(noise, cv::Scalar::all(minVal), cv::Scalar::all(maxVal));
        noise.convertTo(image, CV_8U);
    }

    return image;
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ImageGeneratorModel::save() const
{
    QJsonObject modelJson;
    modelJson["pattern"] = static_cast<int>(m_pattern);
    modelJson["width"] = m_width;
    modelJson["height"] = m_height;
    modelJson["channels"] = m_channels;
    modelJson["value1"] = m_value1;
    modelJson["value2"] = m_value2;
    modelJson["random"] = m_random;
    return modelJson;
}

void ImageGeneratorModel::load(QJsonObject const& model)
{
    std::fflush(stderr);

    QJsonValue patternJson = model["pattern"];
    if (!patternJson.isUndefined())
    {
        int pattern = patternJson.toInt();
        std::fflush(stderr);

        // Also update member variable directly
        m_pattern = static_cast<PatternType>(pattern);
        std::fflush(stderr);

        bool found = false;
        for (int i = 0; i < m_patternCombo->count(); ++i)
        {
            int itemPattern = m_patternCombo->itemData(i).toInt();
            std::fflush(stderr);

            if (itemPattern == pattern)
            {
                std::fflush(stderr);
                m_patternCombo->blockSignals(true);
                m_patternCombo->setCurrentIndex(i);
                m_patternCombo->blockSignals(false);
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::fflush(stderr);
        }
    }
    else
    {
        std::fflush(stderr);
    }

    QJsonValue widthJson = model["width"];
    if (!widthJson.isUndefined())
    {
        m_width = widthJson.toInt();
        m_widthSpin->setValue(m_width);
        std::fflush(stderr);
    }

    QJsonValue heightJson = model["height"];
    if (!heightJson.isUndefined())
    {
        m_height = heightJson.toInt();
        m_heightSpin->setValue(m_height);
        std::fflush(stderr);
    }

    QJsonValue channelsJson = model["channels"];
    if (!channelsJson.isUndefined())
    {
        int channels = channelsJson.toInt();
        for (int i = 0; i < m_channelsCombo->count(); ++i)
        {
            if (m_channelsCombo->itemData(i).toInt() == channels)
            {
                m_channelsCombo->blockSignals(true);
                m_channelsCombo->setCurrentIndex(i);
                m_channelsCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue value1Json = model["value1"];
    if (!value1Json.isUndefined())
    {
        m_value1 = value1Json.toDouble();
        m_value1Spin->setValue(m_value1);
        std::fflush(stderr);
    }

    QJsonValue value2Json = model["value2"];
    if (!value2Json.isUndefined())
    {
        m_value2 = value2Json.toDouble();
        m_value2Spin->setValue(m_value2);
        std::fflush(stderr);
    }

    QJsonValue randomJson = model["random"];
    if (!randomJson.isUndefined())
    {
        m_random = randomJson.toBool();
        m_randomCheck->setChecked(m_random);
        std::fflush(stderr);
    }

    std::fflush(stderr);
    generateImage();

    // Emit signal to trigger downstream nodes
    Q_EMIT dataUpdated(0);
}

} // namespace VisionBox
