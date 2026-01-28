/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Rotate Transform Node Model Implementation
 ******************************************************************************/

#include "RotateModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
RotateModel::RotateModel()
    : m_outputImage(nullptr)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Angle input
    auto* angleLayout = new QHBoxLayout();
    angleLayout->addWidget(new QLabel("Angle (deg):"));
    m_angleSpin = new QDoubleSpinBox();
    m_angleSpin->setRange(-360.0, 360.0);
    m_angleSpin->setSingleStep(1.0);
    m_angleSpin->setValue(0.0);
    angleLayout->addWidget(m_angleSpin);
    layout->addLayout(angleLayout);

    // Interpolation mode selector
    auto* interpLayout = new QHBoxLayout();
    interpLayout->addWidget(new QLabel("Interpolation:"));
    m_interpolationCombo = new QComboBox();
    m_interpolationCombo->addItem("Nearest Neighbor", Nearest);
    m_interpolationCombo->addItem("Bilinear", Linear);
    m_interpolationCombo->addItem("Bicubic", Cubic);
    m_interpolationCombo->addItem("Pixel Area Relation", Area);
    m_interpolationCombo->addItem("Lanczos4", Lanczos4);
    m_interpolationCombo->setCurrentIndex(static_cast<int>(Linear));
    interpLayout->addWidget(m_interpolationCombo);
    layout->addLayout(interpLayout);

    // Expand image checkbox
    m_expandCheck = new QCheckBox("Expand Image");
    m_expandCheck->setToolTip("Expand output image to fit entire rotated image");
    layout->addWidget(m_expandCheck);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_angleSpin, &QDoubleSpinBox::valueChanged,
            this, &RotateModel::onAngleChanged);
    connect(m_interpolationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RotateModel::onInterpolationChanged);
    connect(m_expandCheck, &QCheckBox::stateChanged,
            this, &RotateModel::onExpandChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int RotateModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType RotateModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> RotateModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void RotateModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                            QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    applyRotation();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* RotateModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void RotateModel::applyRotation()
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
        // Get the rotation matrix
        cv::Point2f center(input.cols / 2.0, input.rows / 2.0);
        cv::Mat rotMatrix = cv::getRotationMatrix2D(center, m_angle, 1.0);

        // Map interpolation mode to OpenCV flag
        int interpFlag;
        switch (m_interpolation)
        {
            case Nearest:
                interpFlag = cv::INTER_NEAREST;
                break;
            case Linear:
                interpFlag = cv::INTER_LINEAR;
                break;
            case Cubic:
                interpFlag = cv::INTER_CUBIC;
                break;
            case Area:
                interpFlag = cv::INTER_AREA;
                break;
            case Lanczos4:
                interpFlag = cv::INTER_LANCZOS4;
                break;
            default:
                interpFlag = cv::INTER_LINEAR;
                break;
        }

        cv::Mat output;
        if (m_expand)
        {
            // Calculate bounding box of the rotated image
            cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), input.size(), m_angle).boundingRect2f();

            // Adjust transformation matrix
            rotMatrix.at<double>(0, 2) += bbox.width / 2.0 - input.cols / 2.0;
            rotMatrix.at<double>(1, 2) += bbox.height / 2.0 - input.rows / 2.0;

            // Apply affine transformation
            cv::warpAffine(input, output, rotMatrix, bbox.size(),
                          interpFlag, cv::BORDER_CONSTANT,
                          cv::Scalar());
        }
        else
        {
            // Apply rotation without expanding output size
            cv::warpAffine(input, output, rotMatrix, input.size(),
                          interpFlag, cv::BORDER_CONSTANT,
                          cv::Scalar());
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

void RotateModel::onAngleChanged(double value)
{
    m_angle = value;
    applyRotation();
}

void RotateModel::onInterpolationChanged(int index)
{
    m_interpolation = static_cast<InterpolationMode>(
        m_interpolationCombo->itemData(index).toInt()
    );
    applyRotation();
}

void RotateModel::onExpandChanged(int state)
{
    m_expand = (state == Qt::Checked);
    applyRotation();
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject RotateModel::save() const
{
    QJsonObject modelJson;
    modelJson["angle"] = m_angle;
    modelJson["interpolation"] = static_cast<int>(m_interpolation);
    modelJson["expand"] = m_expand;
    return modelJson;
}

void RotateModel::load(QJsonObject const& model)
{
    QJsonValue angleJson = model["angle"];
    if (!angleJson.isUndefined())
    {
        m_angle = angleJson.toDouble();
        m_angleSpin->setValue(m_angle);
    }

    QJsonValue interpJson = model["interpolation"];
    if (!interpJson.isUndefined())
    {
        m_interpolation = static_cast<InterpolationMode>(interpJson.toInt());
        m_interpolationCombo->setCurrentIndex(static_cast<int>(m_interpolation));
    }

    QJsonValue expandJson = model["expand"];
    if (!expandJson.isUndefined())
    {
        m_expand = expandJson.toBool();
        m_expandCheck->setChecked(m_expand);
    }

    applyRotation();
}

} // namespace VisionBox
