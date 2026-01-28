/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Perspective Transform Model Implementation
 ******************************************************************************/

#include "PerspectiveTransformModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
PerspectiveTransformModel::PerspectiveTransformModel()
    : m_outputImage(nullptr)
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Source points
    auto* srcLabel = new QLabel("Source Points (corners):");
    srcLabel->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addWidget(srcLabel);

    auto* srcPoint1Layout = new QHBoxLayout();
    srcPoint1Layout->addWidget(new QLabel("TL:"));
    m_srcX1Spin = new QDoubleSpinBox();
    m_srcX1Spin->setRange(0.0, 4096.0);
    m_srcX1Spin->setValue(0.0);
    srcPoint1Layout->addWidget(m_srcX1Spin);
    m_srcY1Spin = new QDoubleSpinBox();
    m_srcY1Spin->setRange(0.0, 4096.0);
    m_srcY1Spin->setValue(0.0);
    srcPoint1Layout->addWidget(m_srcY1Spin);
    layout->addLayout(srcPoint1Layout);

    auto* srcPoint2Layout = new QHBoxLayout();
    srcPoint2Layout->addWidget(new QLabel("TR:"));
    m_srcX2Spin = new QDoubleSpinBox();
    m_srcX2Spin->setRange(0.0, 4096.0);
    m_srcX2Spin->setValue(100.0);
    srcPoint2Layout->addWidget(m_srcX2Spin);
    m_srcY2Spin = new QDoubleSpinBox();
    m_srcY2Spin->setRange(0.0, 4096.0);
    m_srcY2Spin->setValue(0.0);
    srcPoint2Layout->addWidget(m_srcY2Spin);
    layout->addLayout(srcPoint2Layout);

    auto* srcPoint3Layout = new QHBoxLayout();
    srcPoint3Layout->addWidget(new QLabel("BR:"));
    m_srcX3Spin = new QDoubleSpinBox();
    m_srcX3Spin->setRange(0.0, 4096.0);
    m_srcX3Spin->setValue(100.0);
    srcPoint3Layout->addWidget(m_srcX3Spin);
    m_srcY3Spin = new QDoubleSpinBox();
    m_srcY3Spin->setRange(0.0, 4096.0);
    m_srcY3Spin->setValue(100.0);
    srcPoint3Layout->addWidget(m_srcY3Spin);
    layout->addLayout(srcPoint3Layout);

    auto* srcPoint4Layout = new QHBoxLayout();
    srcPoint4Layout->addWidget(new QLabel("BL:"));
    m_srcX4Spin = new QDoubleSpinBox();
    m_srcX4Spin->setRange(0.0, 4096.0);
    m_srcX4Spin->setValue(0.0);
    srcPoint4Layout->addWidget(m_srcX4Spin);
    m_srcY4Spin = new QDoubleSpinBox();
    m_srcY4Spin->setRange(0.0, 4096.0);
    m_srcY4Spin->setValue(100.0);
    srcPoint4Layout->addWidget(m_srcY4Spin);
    layout->addLayout(srcPoint4Layout);

    // Destination points
    auto* dstLabel = new QLabel("Destination Points:");
    dstLabel->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addWidget(dstLabel);

    auto* dstPoint1Layout = new QHBoxLayout();
    dstPoint1Layout->addWidget(new QLabel("TL:"));
    m_dstX1Spin = new QDoubleSpinBox();
    m_dstX1Spin->setRange(0.0, 4096.0);
    m_dstX1Spin->setValue(0.0);
    dstPoint1Layout->addWidget(m_dstX1Spin);
    m_dstY1Spin = new QDoubleSpinBox();
    m_dstY1Spin->setRange(0.0, 4096.0);
    m_dstY1Spin->setValue(0.0);
    dstPoint1Layout->addWidget(m_dstY1Spin);
    layout->addLayout(dstPoint1Layout);

    auto* dstPoint2Layout = new QHBoxLayout();
    dstPoint2Layout->addWidget(new QLabel("TR:"));
    m_dstX2Spin = new QDoubleSpinBox();
    m_dstX2Spin->setRange(0.0, 4096.0);
    m_dstX2Spin->setValue(100.0);
    dstPoint2Layout->addWidget(m_dstX2Spin);
    m_dstY2Spin = new QDoubleSpinBox();
    m_dstY2Spin->setRange(0.0, 4096.0);
    m_dstY2Spin->setValue(0.0);
    dstPoint2Layout->addWidget(m_dstY2Spin);
    layout->addLayout(dstPoint2Layout);

    auto* dstPoint3Layout = new QHBoxLayout();
    dstPoint3Layout->addWidget(new QLabel("BR:"));
    m_dstX3Spin = new QDoubleSpinBox();
    m_dstX3Spin->setRange(0.0, 4096.0);
    m_dstX3Spin->setValue(100.0);
    dstPoint3Layout->addWidget(m_dstX3Spin);
    m_dstY3Spin = new QDoubleSpinBox();
    m_dstY3Spin->setRange(0.0, 4096.0);
    m_dstY3Spin->setValue(100.0);
    dstPoint3Layout->addWidget(m_dstY3Spin);
    layout->addLayout(dstPoint3Layout);

    auto* dstPoint4Layout = new QHBoxLayout();
    dstPoint4Layout->addWidget(new QLabel("BL:"));
    m_dstX4Spin = new QDoubleSpinBox();
    m_dstX4Spin->setRange(0.0, 4096.0);
    m_dstX4Spin->setValue(0.0);
    dstPoint4Layout->addWidget(m_dstX4Spin);
    m_dstY4Spin = new QDoubleSpinBox();
    m_dstY4Spin->setRange(0.0, 4096.0);
    m_dstY4Spin->setValue(100.0);
    dstPoint4Layout->addWidget(m_dstY4Spin);
    layout->addLayout(dstPoint4Layout);

    // Output size
    auto* sizeLayout = new QHBoxLayout();
    m_outputWidthSpin = new QSpinBox();
    m_outputWidthSpin->setRange(64, 4096);
    m_outputWidthSpin->setValue(640);
    sizeLayout->addWidget(new QLabel("W:"));
    sizeLayout->addWidget(m_outputWidthSpin);
    m_outputHeightSpin = new QSpinBox();
    m_outputHeightSpin->setRange(64, 4096);
    m_outputHeightSpin->setValue(480);
    sizeLayout->addWidget(new QLabel("H:"));
    sizeLayout->addWidget(m_outputHeightSpin);
    layout->addLayout(sizeLayout);

    // Interpolation
    auto* interpLayout = new QHBoxLayout();
    interpLayout->addWidget(new QLabel("Interpolation:"));
    m_interpolationCombo = new QComboBox();
    m_interpolationCombo->addItem("Nearest", cv::INTER_NEAREST);
    m_interpolationCombo->addItem("Linear", cv::INTER_LINEAR);
    m_interpolationCombo->addItem("Cubic", cv::INTER_CUBIC);
    m_interpolationCombo->addItem("Lanczos", cv::INTER_LANCZOS4);
    m_interpolationCombo->setCurrentIndex(1); // Linear
    m_interpolationCombo->setMinimumWidth(100);
    interpLayout->addWidget(m_interpolationCombo);
    layout->addLayout(interpLayout);

    // Border mode
    auto* borderLayout = new QHBoxLayout();
    borderLayout->addWidget(new QLabel("Border Mode:"));
    m_borderModeCombo = new QComboBox();
    m_borderModeCombo->addItem("Constant", cv::BORDER_CONSTANT);
    m_borderModeCombo->addItem("Replicate", cv::BORDER_REPLICATE);
    m_borderModeCombo->addItem("Reflect", cv::BORDER_REFLECT);
    m_borderModeCombo->addItem("Wrap", cv::BORDER_WRAP);
    m_borderModeCombo->setCurrentIndex(0);
    m_borderModeCombo->setMinimumWidth(120);
    borderLayout->addWidget(m_borderModeCombo);
    layout->addLayout(borderLayout);

    // Border value
    auto* borderValueLayout = new QHBoxLayout();
    borderValueLayout->addWidget(new QLabel("Border Value:"));
    m_borderValueSpin = new QDoubleSpinBox();
    m_borderValueSpin->setRange(0.0, 255.0);
    m_borderValueSpin->setSingleStep(1.0);
    m_borderValueSpin->setValue(0.0);
    borderValueLayout->addWidget(m_borderValueSpin);
    layout->addLayout(borderValueLayout);

    // Auto rect
    m_autoRectCheck = new QCheckBox("Auto-Adjust to Image Size");
    m_autoRectCheck->setToolTip("Automatically set source points to image corners");
    m_autoRectCheck->setChecked(true);
    layout->addWidget(m_autoRectCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_srcX1Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_srcY1Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_srcX2Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_srcY2Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_srcX3Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_srcY3Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_srcX4Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_srcY4Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);

    connect(m_dstX1Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_dstY1Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_dstX2Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_dstY2Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_dstX3Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_dstY3Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_dstX4Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);
    connect(m_dstY4Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onPointChanged);

    connect(m_outputWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onOutputWidthChanged);
    connect(m_outputHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onOutputHeightChanged);
    connect(m_interpolationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PerspectiveTransformModel::onInterpolationChanged);
    connect(m_borderModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PerspectiveTransformModel::onBorderModeChanged);
    connect(m_borderValueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PerspectiveTransformModel::onBorderValueChanged);
    connect(m_autoRectCheck, &QCheckBox::stateChanged,
            this, &PerspectiveTransformModel::onAutoRectChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int PerspectiveTransformModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image
    }
    else
    {
        return 1; // One output image
    }
}

QtNodes::NodeDataType PerspectiveTransformModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> PerspectiveTransformModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void PerspectiveTransformModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                          QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    if (m_inputImage && !m_inputImage->image().empty())
    {
        m_inputWidth = m_inputImage->image().cols;
        m_inputHeight = m_inputImage->image().rows;

        if (m_autoRect)
        {
            updateDefaultPoints();
        }
    }

    applyTransform();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* PerspectiveTransformModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void PerspectiveTransformModel::onPointChanged()
{
    m_srcX1 = m_srcX1Spin->value();
    m_srcY1 = m_srcY1Spin->value();
    m_srcX2 = m_srcX2Spin->value();
    m_srcY2 = m_srcY2Spin->value();
    m_srcX3 = m_srcX3Spin->value();
    m_srcY3 = m_srcY3Spin->value();
    m_srcX4 = m_srcX4Spin->value();
    m_srcY4 = m_srcY4Spin->value();

    m_dstX1 = m_dstX1Spin->value();
    m_dstY1 = m_dstY1Spin->value();
    m_dstX2 = m_dstX2Spin->value();
    m_dstY2 = m_dstY2Spin->value();
    m_dstX3 = m_dstX3Spin->value();
    m_dstY3 = m_dstY3Spin->value();
    m_dstX4 = m_dstX4Spin->value();
    m_dstY4 = m_dstY4Spin->value();

    applyTransform();
}

void PerspectiveTransformModel::onOutputWidthChanged(int value)
{
    m_outputWidth = value;
    applyTransform();
}

void PerspectiveTransformModel::onOutputHeightChanged(int value)
{
    m_outputHeight = value;
    applyTransform();
}

void PerspectiveTransformModel::onInterpolationChanged()
{
    m_interpolation = m_interpolationCombo->currentData().toInt();
    applyTransform();
}

void PerspectiveTransformModel::onBorderModeChanged()
{
    m_borderMode = m_borderModeCombo->currentData().toInt();
    applyTransform();
}

void PerspectiveTransformModel::onBorderValueChanged(double value)
{
    m_borderValue = value;
    applyTransform();
}

void PerspectiveTransformModel::onAutoRectChanged(int state)
{
    m_autoRect = (state == Qt::Checked);

    if (m_autoRect)
    {
        updateDefaultPoints();
    }
}

/*******************************************************************************
 * Perspective Transform
 ******************************************************************************/
void PerspectiveTransformModel::applyTransform()
{
    if (!m_inputImage)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
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
        // Define source points
        std::vector<cv::Point2f> srcPoints = {
            cv::Point2f(static_cast<float>(m_srcX1), static_cast<float>(m_srcY1)),
            cv::Point2f(static_cast<float>(m_srcX2), static_cast<float>(m_srcY2)),
            cv::Point2f(static_cast<float>(m_srcX3), static_cast<float>(m_srcY3)),
            cv::Point2f(static_cast<float>(m_srcX4), static_cast<float>(m_srcY4))
        };

        // Define destination points
        std::vector<cv::Point2f> dstPoints = {
            cv::Point2f(static_cast<float>(m_dstX1), static_cast<float>(m_dstY1)),
            cv::Point2f(static_cast<float>(m_dstX2), static_cast<float>(m_dstY2)),
            cv::Point2f(static_cast<float>(m_dstX3), static_cast<float>(m_dstY3)),
            cv::Point2f(static_cast<float>(m_dstX4), static_cast<float>(m_dstY4))
        };

        // Calculate perspective transform matrix
        cv::Mat matrix = cv::getPerspectiveTransform(srcPoints, dstPoints);

        // Apply perspective transformation
        cv::Size outputSize(m_outputWidth, m_outputHeight);
        cv::Mat output;
        cv::warpPerspective(input, output, matrix, outputSize,
                           m_interpolation, m_borderMode,
                           cv::Scalar(m_borderValue));

        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void PerspectiveTransformModel::updateDefaultPoints()
{
    if (m_inputWidth == 0 || m_inputHeight == 0)
    {
        return;
    }

    // Set source points to image corners
    m_srcX1Spin->blockSignals(true);
    m_srcY1Spin->blockSignals(true);
    m_srcX2Spin->blockSignals(true);
    m_srcY2Spin->blockSignals(true);
    m_srcX3Spin->blockSignals(true);
    m_srcY3Spin->blockSignals(true);
    m_srcX4Spin->blockSignals(true);
    m_srcY4Spin->blockSignals(true);

    m_srcX1Spin->setValue(0.0);
    m_srcY1Spin->setValue(0.0);
    m_srcX2Spin->setValue(static_cast<double>(m_inputWidth));
    m_srcY2Spin->setValue(0.0);
    m_srcX3Spin->setValue(static_cast<double>(m_inputWidth));
    m_srcY3Spin->setValue(static_cast<double>(m_inputHeight));
    m_srcX4Spin->setValue(0.0);
    m_srcY4Spin->setValue(static_cast<double>(m_inputHeight));

    m_srcX1Spin->blockSignals(false);
    m_srcY1Spin->blockSignals(false);
    m_srcX2Spin->blockSignals(false);
    m_srcY2Spin->blockSignals(false);
    m_srcX3Spin->blockSignals(false);
    m_srcY3Spin->blockSignals(false);
    m_srcX4Spin->blockSignals(false);
    m_srcY4Spin->blockSignals(false);

    // Update destination points to match output size
    m_dstX1Spin->blockSignals(true);
    m_dstY1Spin->blockSignals(true);
    m_dstX2Spin->blockSignals(true);
    m_dstY2Spin->blockSignals(true);
    m_dstX3Spin->blockSignals(true);
    m_dstY3Spin->blockSignals(true);
    m_dstX4Spin->blockSignals(true);
    m_dstY4Spin->blockSignals(true);

    m_dstX1Spin->setValue(0.0);
    m_dstY1Spin->setValue(0.0);
    m_dstX2Spin->setValue(static_cast<double>(m_outputWidth));
    m_dstY2Spin->setValue(0.0);
    m_dstX3Spin->setValue(static_cast<double>(m_outputWidth));
    m_dstY3Spin->setValue(static_cast<double>(m_outputHeight));
    m_dstX4Spin->setValue(0.0);
    m_dstY4Spin->setValue(static_cast<double>(m_outputHeight));

    m_dstX1Spin->blockSignals(false);
    m_dstY1Spin->blockSignals(false);
    m_dstX2Spin->blockSignals(false);
    m_dstY2Spin->blockSignals(false);
    m_dstX3Spin->blockSignals(false);
    m_dstY3Spin->blockSignals(false);
    m_dstX4Spin->blockSignals(false);
    m_dstY4Spin->blockSignals(false);

    applyTransform();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject PerspectiveTransformModel::save() const
{
    QJsonObject modelJson;
    modelJson["srcX1"] = m_srcX1;
    modelJson["srcY1"] = m_srcY1;
    modelJson["srcX2"] = m_srcX2;
    modelJson["srcY2"] = m_srcY2;
    modelJson["srcX3"] = m_srcX3;
    modelJson["srcY3"] = m_srcY3;
    modelJson["srcX4"] = m_srcX4;
    modelJson["srcY4"] = m_srcY4;
    modelJson["dstX1"] = m_dstX1;
    modelJson["dstY1"] = m_dstY1;
    modelJson["dstX2"] = m_dstX2;
    modelJson["dstY2"] = m_dstY2;
    modelJson["dstX3"] = m_dstX3;
    modelJson["dstY3"] = m_dstY3;
    modelJson["dstX4"] = m_dstX4;
    modelJson["dstY4"] = m_dstY4;
    modelJson["outputWidth"] = m_outputWidth;
    modelJson["outputHeight"] = m_outputHeight;
    modelJson["interpolation"] = m_interpolation;
    modelJson["borderMode"] = m_borderMode;
    modelJson["borderValue"] = m_borderValue;
    modelJson["autoRect"] = m_autoRect;
    return modelJson;
}

void PerspectiveTransformModel::load(QJsonObject const& model)
{
    QJsonValue srcX1Json = model["srcX1"];
    if (!srcX1Json.isUndefined())
    {
        m_srcX1 = srcX1Json.toDouble();
        m_srcX1Spin->setValue(m_srcX1);
    }

    QJsonValue srcY1Json = model["srcY1"];
    if (!srcY1Json.isUndefined())
    {
        m_srcY1 = srcY1Json.toDouble();
        m_srcY1Spin->setValue(m_srcY1);
    }

    QJsonValue srcX2Json = model["srcX2"];
    if (!srcX2Json.isUndefined())
    {
        m_srcX2 = srcX2Json.toDouble();
        m_srcX2Spin->setValue(m_srcX2);
    }

    QJsonValue srcY2Json = model["srcY2"];
    if (!srcY2Json.isUndefined())
    {
        m_srcY2 = srcY2Json.toDouble();
        m_srcY2Spin->setValue(m_srcY2);
    }

    QJsonValue srcX3Json = model["srcX3"];
    if (!srcX3Json.isUndefined())
    {
        m_srcX3 = srcX3Json.toDouble();
        m_srcX3Spin->setValue(m_srcX3);
    }

    QJsonValue srcY3Json = model["srcY3"];
    if (!srcY3Json.isUndefined())
    {
        m_srcY3 = srcY3Json.toDouble();
        m_srcY3Spin->setValue(m_srcY3);
    }

    QJsonValue srcX4Json = model["srcX4"];
    if (!srcX4Json.isUndefined())
    {
        m_srcX4 = srcX4Json.toDouble();
        m_srcX4Spin->setValue(m_srcX4);
    }

    QJsonValue srcY4Json = model["srcY4"];
    if (!srcY4Json.isUndefined())
    {
        m_srcY4 = srcY4Json.toDouble();
        m_srcY4Spin->setValue(m_srcY4);
    }

    QJsonValue dstX1Json = model["dstX1"];
    if (!dstX1Json.isUndefined())
    {
        m_dstX1 = dstX1Json.toDouble();
        m_dstX1Spin->setValue(m_dstX1);
    }

    QJsonValue dstY1Json = model["dstY1"];
    if (!dstY1Json.isUndefined())
    {
        m_dstY1 = dstY1Json.toDouble();
        m_dstY1Spin->setValue(m_dstY1);
    }

    QJsonValue dstX2Json = model["dstX2"];
    if (!dstX2Json.isUndefined())
    {
        m_dstX2 = dstX2Json.toDouble();
        m_dstX2Spin->setValue(m_dstX2);
    }

    QJsonValue dstY2Json = model["dstY2"];
    if (!dstY2Json.isUndefined())
    {
        m_dstY2 = dstY2Json.toDouble();
        m_dstY2Spin->setValue(m_dstY2);
    }

    QJsonValue dstX3Json = model["dstX3"];
    if (!dstX3Json.isUndefined())
    {
        m_dstX3 = dstX3Json.toDouble();
        m_dstX3Spin->setValue(m_dstX3);
    }

    QJsonValue dstY3Json = model["dstY3"];
    if (!dstY3Json.isUndefined())
    {
        m_dstY3 = dstY3Json.toDouble();
        m_dstY3Spin->setValue(m_dstY3);
    }

    QJsonValue dstX4Json = model["dstX4"];
    if (!dstX4Json.isUndefined())
    {
        m_dstX4 = dstX4Json.toDouble();
        m_dstX4Spin->setValue(m_dstX4);
    }

    QJsonValue dstY4Json = model["dstY4"];
    if (!dstY4Json.isUndefined())
    {
        m_dstY4 = dstY4Json.toDouble();
        m_dstY4Spin->setValue(m_dstY4);
    }

    QJsonValue outputWidthJson = model["outputWidth"];
    if (!outputWidthJson.isUndefined())
    {
        m_outputWidth = outputWidthJson.toInt();
        m_outputWidthSpin->setValue(m_outputWidth);
    }

    QJsonValue outputHeightJson = model["outputHeight"];
    if (!outputHeightJson.isUndefined())
    {
        m_outputHeight = outputHeightJson.toInt();
        m_outputHeightSpin->setValue(m_outputHeight);
    }

    QJsonValue interpolationJson = model["interpolation"];
    if (!interpolationJson.isUndefined())
    {
        m_interpolation = interpolationJson.toInt();
        for (int i = 0; i < m_interpolationCombo->count(); ++i)
        {
            if (m_interpolationCombo->itemData(i).toInt() == m_interpolation)
            {
                m_interpolationCombo->blockSignals(true);
                m_interpolationCombo->setCurrentIndex(i);
                m_interpolationCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue borderModeJson = model["borderMode"];
    if (!borderModeJson.isUndefined())
    {
        m_borderMode = borderModeJson.toInt();
        for (int i = 0; i < m_borderModeCombo->count(); ++i)
        {
            if (m_borderModeCombo->itemData(i).toInt() == m_borderMode)
            {
                m_borderModeCombo->blockSignals(true);
                m_borderModeCombo->setCurrentIndex(i);
                m_borderModeCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue borderValueJson = model["borderValue"];
    if (!borderValueJson.isUndefined())
    {
        m_borderValue = borderValueJson.toDouble();
        m_borderValueSpin->setValue(m_borderValue);
    }

    QJsonValue autoRectJson = model["autoRect"];
    if (!autoRectJson.isUndefined())
    {
        m_autoRect = autoRectJson.toBool();
        m_autoRectCheck->setChecked(m_autoRect);
    }

    applyTransform();
}

} // namespace VisionBox
