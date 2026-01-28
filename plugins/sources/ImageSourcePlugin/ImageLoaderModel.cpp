/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Loader Node Model Implementation
 ******************************************************************************/

#include "ImageLoaderModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
ImageLoaderModel::ImageLoaderModel()
    : m_imageData(nullptr)
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    m_pathLabel = new QLabel("No image loaded");
    m_pathLabel->setWordWrap(true);
    m_pathLabel->setStyleSheet("QLabel { padding: 5px; }");

    m_browseButton = new QPushButton("Browse...");
    m_browseButton->setStyleSheet("QPushButton { padding: 5px; }");

    layout->addWidget(m_pathLabel);
    layout->addWidget(m_browseButton);
    layout->setContentsMargins(5, 5, 5, 5);

    connect(m_browseButton, &QPushButton::clicked,
            this, &ImageLoaderModel::onBrowseClicked);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ImageLoaderModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 0; // No input ports
    }
    else
    {
        return 1; // One output port for the image
    }
}

QtNodes::NodeDataType ImageLoaderModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ImageLoaderModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_imageData;
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ImageLoaderModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ImageLoaderModel::onBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open Image",
        "",
        "Image Files (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp);;All Files (*.*)"
    );

    if (!filePath.isEmpty())
    {
        loadImage(filePath);
    }
}

void ImageLoaderModel::loadImage(const QString& filePath)
{
    m_filePath = filePath;

    // Load image using OpenCV
    cv::Mat image = cv::imread(filePath.toStdString());
    if (image.empty())
    {
        m_pathLabel->setText("Failed to load: " + filePath);
        m_imageData = nullptr;
        Q_EMIT dataUpdated(0);
        return;
    }

    // Create ImageData
    m_imageData = std::make_shared<ImageData>(image);

    // Update UI
    m_pathLabel->setText("Loaded: " + QFileInfo(filePath).fileName());

    // Notify that data has changed
    Q_EMIT dataUpdated(0);
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ImageLoaderModel::save() const
{
    QJsonObject modelJson;
    modelJson["filePath"] = m_filePath;
    return modelJson;
}

void ImageLoaderModel::load(QJsonObject const& model)
{
    QJsonValue filePathJson = model["filePath"];
    if (!filePathJson.isUndefined())
    {
        QString filePath = filePathJson.toString();
        if (!filePath.isEmpty() && QFile::exists(filePath))
        {
            loadImage(filePath);
        }
    }
}

} // namespace VisionBox
