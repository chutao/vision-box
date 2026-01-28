/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Affine Transform Model Implementation
 ******************************************************************************/

#include "AffineTransformModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
AffineTransformModel::AffineTransformModel()
    : m_outputImage(nullptr)
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Rotation
    auto* rotationLayout = new QHBoxLayout();
    rotationLayout->addWidget(new QLabel("Rotation (deg):"));
    m_rotationSpin = new QDoubleSpinBox();
    m_rotationSpin->setRange(-180.0, 180.0);
    m_rotationSpin->setSingleStep(1.0);
    m_rotationSpin->setValue(0.0);
    rotationLayout->addWidget(m_rotationSpin);
    layout->addLayout(rotationLayout);

    // Scale X
    auto* scaleXLayout = new QHBoxLayout();
    scaleXLayout->addWidget(new QLabel("Scale X:"));
    m_scaleXSpin = new QDoubleSpinBox();
    m_scaleXSpin->setRange(0.1, 10.0);
    m_scaleXSpin->setSingleStep(0.1);
    m_scaleXSpin->setValue(1.0);
    scaleXLayout->addWidget(m_scaleXSpin);
    layout->addLayout(scaleXLayout);

    // Scale Y
    auto* scaleYLayout = new QHBoxLayout();
    scaleYLayout->addWidget(new QLabel("Scale Y:"));
    m_scaleYSpin = new QDoubleSpinBox();
    m_scaleYSpin->setRange(0.1, 10.0);
    m_scaleYSpin->setSingleStep(0.1);
    m_scaleYSpin->setValue(1.0);
    scaleYLayout->addWidget(m_scaleYSpin);
    layout->addLayout(scaleYLayout);

    // Translation X
    auto* transXLayout = new QHBoxLayout();
    transXLayout->addWidget(new QLabel("Translate X:"));
    m_translationXSpin = new QDoubleSpinBox();
    m_translationXSpin->setRange(-1000.0, 1000.0);
    m_translationXSpin->setSingleStep(10.0);
    m_translationXSpin->setValue(0.0);
    transXLayout->addWidget(m_translationXSpin);
    layout->addLayout(transXLayout);

    // Translation Y
    auto* transYLayout = new QHBoxLayout();
    transYLayout->addWidget(new QLabel("Translate Y:"));
    m_translationYSpin = new QDoubleSpinBox();
    m_translationYSpin->setRange(-1000.0, 1000.0);
    m_translationYSpin->setSingleStep(10.0);
    m_translationYSpin->setValue(0.0);
    transYLayout->addWidget(m_translationYSpin);
    layout->addLayout(transYLayout);

    // Shear X
    auto* shearXLayout = new QHBoxLayout();
    shearXLayout->addWidget(new QLabel("Shear X:"));
    m_shearXSpin = new QDoubleSpinBox();
    m_shearXSpin->setRange(-5.0, 5.0);
    m_shearXSpin->setSingleStep(0.1);
    m_shearXSpin->setValue(0.0);
    shearXLayout->addWidget(m_shearXSpin);
    layout->addLayout(shearXLayout);

    // Shear Y
    auto* shearYLayout = new QHBoxLayout();
    shearYLayout->addWidget(new QLabel("Shear Y:"));
    m_shearYSpin = new QDoubleSpinBox();
    m_shearYSpin->setRange(-5.0, 5.0);
    m_shearYSpin->setSingleStep(0.1);
    m_shearYSpin->setValue(0.0);
    shearYLayout->addWidget(m_shearYSpin);
    layout->addLayout(shearYLayout);

    // Interpolation
    auto* interpLayout = new QHBoxLayout();
    interpLayout->addWidget(new QLabel("Interpolation:"));
    m_interpolationCombo = new QComboBox();
    m_interpolationCombo->addItem("Nearest", cv::INTER_NEAREST);
    m_interpolationCombo->addItem("Linear", cv::INTER_LINEAR);
    m_interpolationCombo->addItem("Cubic", cv::INTER_CUBIC);
    m_interpolationCombo->addItem("Lanczos", cv::INTER_LANCZOS4);
    m_interpolationCombo->addItem("Area", cv::INTER_AREA);
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
    m_borderModeCombo->addItem("Transparent", cv::BORDER_TRANSPARENT);
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

    // Invert
    m_invertCheck = new QCheckBox("Invert Transform");
    m_invertCheck->setToolTip("Apply inverse transformation");
    m_invertCheck->setChecked(false);
    layout->addWidget(m_invertCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_rotationSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onRotationChanged);
    connect(m_scaleXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onScaleXChanged);
    connect(m_scaleYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onScaleYChanged);
    connect(m_translationXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onTranslationXChanged);
    connect(m_translationYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onTranslationYChanged);
    connect(m_shearXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onShearXChanged);
    connect(m_shearYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onShearYChanged);
    connect(m_interpolationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AffineTransformModel::onInterpolationChanged);
    connect(m_borderModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AffineTransformModel::onBorderModeChanged);
    connect(m_borderValueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AffineTransformModel::onBorderValueChanged);
    connect(m_invertCheck, &QCheckBox::stateChanged,
            this, &AffineTransformModel::onInvertChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int AffineTransformModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType AffineTransformModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> AffineTransformModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void AffineTransformModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                     QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    applyTransform();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* AffineTransformModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void AffineTransformModel::onRotationChanged(double value)
{
    m_rotation = value;
    applyTransform();
}

void AffineTransformModel::onScaleXChanged(double value)
{
    m_scaleX = value;
    applyTransform();
}

void AffineTransformModel::onScaleYChanged(double value)
{
    m_scaleY = value;
    applyTransform();
}

void AffineTransformModel::onTranslationXChanged(double value)
{
    m_translationX = value;
    applyTransform();
}

void AffineTransformModel::onTranslationYChanged(double value)
{
    m_translationY = value;
    applyTransform();
}

void AffineTransformModel::onShearXChanged(double value)
{
    m_shearX = value;
    applyTransform();
}

void AffineTransformModel::onShearYChanged(double value)
{
    m_shearY = value;
    applyTransform();
}

void AffineTransformModel::onInterpolationChanged()
{
    m_interpolation = m_interpolationCombo->currentData().toInt();
    applyTransform();
}

void AffineTransformModel::onBorderModeChanged()
{
    m_borderMode = m_borderModeCombo->currentData().toInt();
    applyTransform();
}

void AffineTransformModel::onBorderValueChanged(double value)
{
    m_borderValue = value;
    applyTransform();
}

void AffineTransformModel::onInvertChanged(int state)
{
    m_invert = (state == Qt::Checked);
    applyTransform();
}

/*******************************************************************************
 * Affine Transform
 ******************************************************************************/
void AffineTransformModel::applyTransform()
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
        // Get image center
        cv::Point2f center(input.cols / 2.0f, input.rows / 2.0f);

        // Create affine transformation matrix
        // 1. Rotation and scaling around center
        cv::Mat rotMat = cv::getRotationMatrix2D(center, m_rotation, m_scaleX);

        // 2. Add translation
        rotMat.at<double>(0, 2) += m_translationX;
        rotMat.at<double>(1, 2) += m_translationY;

        // 3. Add shear (modify the matrix directly)
        if (m_shearX != 0.0 || m_shearY != 0.0)
        {
            // Apply shear by modifying the rotation matrix
            rotMat.at<double>(0, 0) += m_shearX;
            rotMat.at<double>(0, 1) += m_shearY;
        }

        // Apply different scale for Y if needed
        if (m_scaleY != m_scaleX)
        {
            rotMat.at<double>(1, 1) *= m_scaleY / m_scaleX;
        }

        // Get output size (same as input)
        cv::Size outputSize(input.cols, input.rows);

        // Apply affine transformation
        cv::Mat output;
        if (m_invert)
        {
            cv::Mat invMat;
            cv::invert(rotMat, invMat);
            cv::warpAffine(input, output, invMat, outputSize,
                           m_interpolation, m_borderMode,
                           cv::Scalar(m_borderValue));
        }
        else
        {
            cv::warpAffine(input, output, rotMat, outputSize,
                           m_interpolation, m_borderMode,
                           cv::Scalar(m_borderValue));
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
QJsonObject AffineTransformModel::save() const
{
    QJsonObject modelJson;
    modelJson["rotation"] = m_rotation;
    modelJson["scaleX"] = m_scaleX;
    modelJson["scaleY"] = m_scaleY;
    modelJson["translationX"] = m_translationX;
    modelJson["translationY"] = m_translationY;
    modelJson["shearX"] = m_shearX;
    modelJson["shearY"] = m_shearY;
    modelJson["interpolation"] = m_interpolation;
    modelJson["borderMode"] = m_borderMode;
    modelJson["borderValue"] = m_borderValue;
    modelJson["invert"] = m_invert;
    return modelJson;
}

void AffineTransformModel::load(QJsonObject const& model)
{
    QJsonValue rotationJson = model["rotation"];
    if (!rotationJson.isUndefined())
    {
        m_rotation = rotationJson.toDouble();
        m_rotationSpin->setValue(m_rotation);
    }

    QJsonValue scaleXJson = model["scaleX"];
    if (!scaleXJson.isUndefined())
    {
        m_scaleX = scaleXJson.toDouble();
        m_scaleXSpin->setValue(m_scaleX);
    }

    QJsonValue scaleYJson = model["scaleY"];
    if (!scaleYJson.isUndefined())
    {
        m_scaleY = scaleYJson.toDouble();
        m_scaleYSpin->setValue(m_scaleY);
    }

    QJsonValue translationXJson = model["translationX"];
    if (!translationXJson.isUndefined())
    {
        m_translationX = translationXJson.toDouble();
        m_translationXSpin->setValue(m_translationX);
    }

    QJsonValue translationYJson = model["translationY"];
    if (!translationYJson.isUndefined())
    {
        m_translationY = translationYJson.toDouble();
        m_translationYSpin->setValue(m_translationY);
    }

    QJsonValue shearXJson = model["shearX"];
    if (!shearXJson.isUndefined())
    {
        m_shearX = shearXJson.toDouble();
        m_shearXSpin->setValue(m_shearX);
    }

    QJsonValue shearYJson = model["shearY"];
    if (!shearYJson.isUndefined())
    {
        m_shearY = shearYJson.toDouble();
        m_shearYSpin->setValue(m_shearY);
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

    QJsonValue invertJson = model["invert"];
    if (!invertJson.isUndefined())
    {
        m_invert = invertJson.toBool();
        m_invertCheck->setChecked(m_invert);
    }

    applyTransform();
}

} // namespace VisionBox
