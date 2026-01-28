/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Viewer Node Model Implementation
 ******************************************************************************/

#include "ImageViewerModel.h"
#include "core/VisionDataTypes.h"
#include "core/PerformanceMonitor.h"
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
ImageViewerModel::ImageViewerModel()
{
    // Create embedded widget
    m_label = new QLabel();
    m_label->setMinimumSize(200, 150);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet(
        "QLabel {"
        "  background-color: #2b2b2b;"
        "  border: 2px solid #555555;"
        "  color: #888888;"
        "  font-size: 12px;"
        "}"
    );
    m_label->setText("No Image");

    auto* layout = new QVBoxLayout(m_label);
    layout->setContentsMargins(0, 0, 0, 0);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ImageViewerModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input port for image
    }
    else
    {
        return 0; // No output ports
    }
}

QtNodes::NodeDataType ImageViewerModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ImageViewerModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return nullptr; // No output
}

void ImageViewerModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                           QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);

    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);
    m_imageChanged = true;
    updateImage();
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ImageViewerModel::embeddedWidget()
{
    return m_label;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ImageViewerModel::updateImage()
{
    if (!m_imageChanged) return;
    m_imageChanged = false;

    if (!m_inputImage)
    {
        m_label->setPixmap(QPixmap());
        m_label->setText("No Image");
        return;
    }

    // Start performance timer
    PerformanceTimer timer(this, caption());

    cv::Mat image = m_inputImage->image();
    if (image.empty())
    {
        m_label->setPixmap(QPixmap());
        m_label->setText("Empty Image");
        return;
    }

    // Convert cv::Mat to QImage
    QImage qImage;
    switch (image.channels())
    {
        case 1: // Grayscale
            qImage = QImage(
                image.data,
                image.cols,
                image.rows,
                static_cast<int>(image.step),
                QImage::Format_Grayscale8
            );
            break;
        case 3: // BGR
            qImage = QImage(
                image.data,
                image.cols,
                image.rows,
                static_cast<int>(image.step),
                QImage::Format_RGB888
            ).rgbSwapped(); // BGR to RGB
            break;
        case 4: // BGRA
            qImage = QImage(
                image.data,
                image.cols,
                image.rows,
                static_cast<int>(image.step),
                QImage::Format_ARGB32
            ).rgbSwapped(); // BGRA to ARGB
            break;
        default:
            m_label->setText("Unsupported\nFormat");
            return;
    }

    // Scale image to fit label while maintaining aspect ratio
    QPixmap pixmap = QPixmap::fromImage(qImage);
    m_label->setPixmap(pixmap.scaled(
        m_label->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    ));

    // Update tooltip with image info
    m_label->setToolTip(QString("Size: %1x%2\nChannels: %3\nDepth: %4")
        .arg(image.cols)
        .arg(image.rows)
        .arg(image.channels())
        .arg(image.depth()));
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ImageViewerModel::save() const
{
    QJsonObject modelJson;
    return modelJson;
}

void ImageViewerModel::load(QJsonObject const& model)
{
    Q_UNUSED(model);
    updateImage();
}

} // namespace VisionBox
