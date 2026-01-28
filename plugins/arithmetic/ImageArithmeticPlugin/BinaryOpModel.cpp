/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Binary Operation Node Model Implementation
 ******************************************************************************/

#include "BinaryOpModel.h"
#include "core/VisionDataTypes.h"
#include "core/PerformanceMonitor.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * BinaryOpModelBase Implementation
 ******************************************************************************/
BinaryOpModelBase::BinaryOpModelBase(const QString& name, const QString& caption,
                                     double alpha, double alphaMin, double alphaMax,
                                     const QString& alphaLabel)
    : m_outputImage(nullptr), m_alpha(alpha), m_name(name), m_caption(caption)
{
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    auto* alphaLayout = new QHBoxLayout();
    alphaLayout->addWidget(new QLabel(alphaLabel));
    m_alphaSpin = new QDoubleSpinBox();
    m_alphaSpin->setRange(alphaMin, alphaMax);
    m_alphaSpin->setSingleStep(0.1);
    m_alphaSpin->setValue(alpha);
    alphaLayout->addWidget(m_alphaSpin);
    layout->addLayout(alphaLayout);

    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_alphaSpin, &QDoubleSpinBox::valueChanged,
            this, &BinaryOpModelBase::onAlphaChanged);
}

unsigned int BinaryOpModelBase::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

QtNodes::NodeDataType BinaryOpModelBase::dataType(
    QtNodes::PortType portType,
    QtNodes::PortIndex portIndex) const
{
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    return ImageData().type();
}

std::shared_ptr<QtNodes::NodeData> BinaryOpModelBase::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_outputImage;
}

void BinaryOpModelBase::setInData(std::shared_ptr<QtNodes::NodeData> data,
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

    applyBinaryOp();
}

QWidget* BinaryOpModelBase::embeddedWidget()
{
    return m_widget;
}

void BinaryOpModelBase::applyBinaryOp()
{
    if (!m_inputImage1 || !m_inputImage2)
    {
        m_outputImage = nullptr;
        return;
    }

    cv::Mat input1 = m_inputImage1->image();
    cv::Mat input2 = m_inputImage2->image();

    if (input1.empty() || input2.empty())
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
        return;
    }

    // Start performance timer
    PerformanceTimer timer(this, caption());

    try
    {
        cv::Mat output = applyOperation(input1, input2, m_alpha);
        m_outputImage = std::make_shared<ImageData>(output);
        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_outputImage = nullptr;
        Q_EMIT dataUpdated(0);
    }
}

void BinaryOpModelBase::onAlphaChanged(double value)
{
    m_alpha = value;
    applyBinaryOp();
}

QJsonObject BinaryOpModelBase::save() const
{
    QJsonObject modelJson;
    modelJson["alpha"] = m_alpha;
    return modelJson;
}

void BinaryOpModelBase::load(QJsonObject const& model)
{
    QJsonValue alphaJson = model["alpha"];
    if (!alphaJson.isUndefined())
    {
        m_alpha = alphaJson.toDouble();
        m_alphaSpin->setValue(m_alpha);
    }

    applyBinaryOp();

    // Emit signal to trigger downstream nodes
    Q_EMIT dataUpdated(0);
}

/*******************************************************************************
 * AddModel Implementation
 ******************************************************************************/
AddModel::AddModel()
    : BinaryOpModelBase("AddModel", "Add", 1.0, 0.0, 10.0, "Scale:")
{
}

cv::Mat AddModel::applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha)
{
    cv::Mat result;
    cv::add(img1, img2, result, cv::noArray(), -1);
    if (alpha != 1.0)
    {
        cv::scaleAdd(result, alpha - 1.0, result, result);
    }
    return result;
}

/*******************************************************************************
 * SubtractModel Implementation
 ******************************************************************************/
SubtractModel::SubtractModel()
    : BinaryOpModelBase("SubtractModel", "Subtract", 1.0, 0.0, 10.0, "Scale:")
{
}

cv::Mat SubtractModel::applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha)
{
    cv::Mat result;
    cv::subtract(img1, img2, result, cv::noArray(), -1);
    if (alpha != 1.0)
    {
        result *= alpha;
    }
    return result;
}

/*******************************************************************************
 * MultiplyModel Implementation
 ******************************************************************************/
MultiplyModel::MultiplyModel()
    : BinaryOpModelBase("MultiplyModel", "Multiply", 1.0, 0.0, 10.0, "Scale:")
{
}

cv::Mat MultiplyModel::applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha)
{
    cv::Mat result;
    cv::multiply(img1, img2, result, alpha);
    return result;
}

/*******************************************************************************
 * DivideModel Implementation
 ******************************************************************************/
DivideModel::DivideModel()
    : BinaryOpModelBase("DivideModel", "Divide", 1.0, 0.1, 10.0, "Scale:")
{
}

cv::Mat DivideModel::applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha)
{
    cv::Mat result;
    cv::divide(img1, img2, result, alpha);
    return result;
}

/*******************************************************************************
 * AbsDiffModel Implementation
 ******************************************************************************/
AbsDiffModel::AbsDiffModel()
    : BinaryOpModelBase("AbsDiffModel", "Abs Diff", 1.0, 0.0, 10.0, "Scale:")
{
}

cv::Mat AbsDiffModel::applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha)
{
    cv::Mat result;
    cv::absdiff(img1, img2, result);
    if (alpha != 1.0)
    {
        result *= alpha;
    }
    return result;
}

/*******************************************************************************
 * BlendModel Implementation
 ******************************************************************************/
BlendModel::BlendModel()
    : BinaryOpModelBase("BlendModel", "Blend", 0.5, 0.0, 1.0, "Weight:")
{
}

cv::Mat BlendModel::applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha)
{
    cv::Mat result;
    cv::addWeighted(img1, 1.0 - alpha, img2, alpha, 0.0, result);
    return result;
}

} // namespace VisionBox
