/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Exporter Model Implementation
 ******************************************************************************/

#include "ImageExporterModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
ImageExporterModel::ImageExporterModel()
{
    // Initialize format mappings
    m_formatExtensions[0] = ".png";
    m_formatExtensions[1] = ".jpg";
    m_formatExtensions[2] = ".jpeg";
    m_formatExtensions[3] = ".bmp";
    m_formatExtensions[4] = ".tiff";
    m_formatExtensions[5] = ".tif";
    m_formatExtensions[6] = ".webp";

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Output directory
    auto* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(new QLabel("Output Dir:"));
    m_pathEdit = new QLineEdit();
    m_pathEdit->setPlaceholderText("/path/to/output");
    pathLayout->addWidget(m_pathEdit);
    m_browseBtn = new QPushButton("Browse...");
    pathLayout->addWidget(m_browseBtn);
    layout->addLayout(pathLayout);

    // File prefix
    auto* prefixLayout = new QHBoxLayout();
    prefixLayout->addWidget(new QLabel("Prefix:"));
    m_prefixEdit = new QLineEdit();
    m_prefixEdit->setPlaceholderText("image");
    prefixLayout->addWidget(m_prefixEdit);
    layout->addLayout(prefixLayout);

    // Format selection
    auto* formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel("Format:"));
    m_formatCombo = new QComboBox();
    m_formatCombo->addItem("PNG (lossless)", 0);
    m_formatCombo->addItem("JPEG", 1);
    m_formatCombo->addItem("JPEG", 2);
    m_formatCombo->addItem("BMP", 3);
    m_formatCombo->addItem("TIFF", 4);
    m_formatCombo->addItem("TIFF", 5);
    m_formatCombo->addItem("WebP", 6);
    m_formatCombo->setCurrentIndex(0);
    m_formatCombo->setMinimumWidth(150);
    formatLayout->addWidget(m_formatCombo);
    layout->addLayout(formatLayout);

    // Quality (for JPEG/WebP)
    auto* qualityLayout = new QHBoxLayout();
    qualityLayout->addWidget(new QLabel("Quality:"));
    m_qualitySpin = new QSpinBox();
    m_qualitySpin->setRange(1, 100);
    m_qualitySpin->setValue(95);
    qualityLayout->addWidget(m_qualitySpin);
    layout->addLayout(qualityLayout);

    // Auto-increment
    m_autoIncrementCheck = new QCheckBox("Auto-Increment Filename");
    m_autoIncrementCheck->setToolTip("Add frame number to filename (e.g., image_001.png)");
    layout->addWidget(m_autoIncrementCheck);

    // Export button
    m_exportBtn = new QPushButton("Export Image");
    m_exportBtn->setEnabled(false);
    layout->addWidget(m_exportBtn);

    // Status label
    m_statusLabel = new QLabel("Status: Ready");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");
    layout->addWidget(m_statusLabel);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_browseBtn, &QPushButton::clicked,
            this, &ImageExporterModel::onBrowseClicked);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &ImageExporterModel::onExportClicked);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageExporterModel::onFormatChanged);
    connect(m_qualitySpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ImageExporterModel::onQualityChanged);
    connect(m_autoIncrementCheck, &QCheckBox::stateChanged,
            this, &ImageExporterModel::onAutoIncrementChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int ImageExporterModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image
    }
    else
    {
        return 0; // No output (sinks don't have outputs)
    }
}

QtNodes::NodeDataType ImageExporterModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> ImageExporterModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return nullptr;
}

void ImageExporterModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                   QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    // Enable export button if we have data
    m_exportBtn->setEnabled(m_inputImage != nullptr && !m_outputPath.isEmpty());
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* ImageExporterModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void ImageExporterModel::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        nullptr,
        "Select Output Directory",
        m_outputPath.isEmpty() ? QDir::homePath() : m_outputPath
    );

    if (!dir.isEmpty())
    {
        m_outputPath = dir;
        m_pathEdit->setText(dir);
        m_exportBtn->setEnabled(m_inputImage != nullptr);
    }
}

void ImageExporterModel::onExportClicked()
{
    if (!m_inputImage)
    {
        m_statusLabel->setText("Status: No image to export");
        return;
    }

    cv::Mat image = m_inputImage->image();
    if (image.empty())
    {
        m_statusLabel->setText("Status: Empty image");
        return;
    }

    if (m_outputPath.isEmpty())
    {
        m_statusLabel->setText("Status: No output directory");
        return;
    }

    // Create output directory if it doesn't exist
    QDir dir;
    if (!dir.exists(m_outputPath))
    {
        if (!dir.mkpath(m_outputPath))
        {
            m_statusLabel->setText("Status: Failed to create directory");
            return;
        }
    }

    // Export image
    if (exportImage(image))
    {
        m_statusLabel->setText(QString("Status: Exported to %1").arg(generateFileName()));

        // Increment frame counter for next export
        if (m_autoIncrement)
        {
            m_frameCount++;
        }
    }
}

void ImageExporterModel::onFormatChanged()
{
    m_formatIndex = m_formatCombo->currentData().toInt();

    // Enable/disable quality spinbox based on format
    bool supportsQuality = (m_formatIndex == 1 || m_formatIndex == 2 || m_formatIndex == 6);
    m_qualitySpin->setEnabled(supportsQuality);
}

void ImageExporterModel::onQualityChanged(int value)
{
    m_quality = value;
}

void ImageExporterModel::onAutoIncrementChanged(int state)
{
    m_autoIncrement = (state == Qt::Checked);
    if (!m_autoIncrement)
    {
        m_frameCount = 0;
    }
}

/*******************************************************************************
 * Image Export
 ******************************************************************************/
bool ImageExporterModel::exportImage(const cv::Mat& image)
{
    QString fileName = generateFileName();
    QString filePath = QDir(m_outputPath).absoluteFilePath(fileName);

    std::vector<int> compression_params;

    // Try to save with appropriate parameters
    try
    {
        switch (m_formatIndex)
        {
            case 0: // PNG
                // PNG doesn't use quality parameter
                cv::imwrite(filePath.toStdString(), image);
                break;

            case 1: // JPEG
            case 2: // JPEG
                compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
                compression_params.push_back(m_quality);
                cv::imwrite(filePath.toStdString(), image, compression_params);
                break;

            case 3: // BMP
                cv::imwrite(filePath.toStdString(), image);
                break;

            case 4: // TIFF
            case 5: // TIFF
                compression_params.push_back(cv::IMWRITE_TIFF_COMPRESSION);
                compression_params.push_back(1); // LZW compression
                cv::imwrite(filePath.toStdString(), image, compression_params);
                break;

            case 6: // WebP
                compression_params.push_back(cv::IMWRITE_WEBP_QUALITY);
                compression_params.push_back(m_quality);
                cv::imwrite(filePath.toStdString(), image, compression_params);
                break;
        }

        return true;
    }
    catch (const cv::Exception& e)
    {
        m_statusLabel->setText(QString("Status: Export failed - %1").arg(e.what()));
        return false;
    }
}

QString ImageExporterModel::generateFileName()
{
    QString prefix = m_prefixEdit->text().isEmpty() ? "image" : m_prefixEdit->text();
    QString extension = m_formatExtensions.value(m_formatIndex, ".png");

    if (m_autoIncrement)
    {
        // Format: prefix_001.ext, prefix_002.ext, etc.
        return QString("%1_%2%3")
            .arg(prefix)
            .arg(m_frameCount, 3, 10, QChar('0'))
            .arg(extension);
    }
    else
    {
        // No auto-increment: prefix.ext
        return QString("%1%2").arg(prefix).arg(extension);
    }
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject ImageExporterModel::save() const
{
    QJsonObject modelJson;
    modelJson["outputPath"] = m_outputPath;
    modelJson["filePrefix"] = m_filePrefix;
    modelJson["formatIndex"] = m_formatIndex;
    modelJson["quality"] = m_quality;
    modelJson["autoIncrement"] = m_autoIncrement;
    modelJson["frameCount"] = m_frameCount;
    return modelJson;
}

void ImageExporterModel::load(QJsonObject const& model)
{
    QJsonValue pathJson = model["outputPath"];
    if (!pathJson.isUndefined())
    {
        m_outputPath = pathJson.toString();
        m_pathEdit->setText(m_outputPath);
    }

    QJsonValue prefixJson = model["filePrefix"];
    if (!prefixJson.isUndefined())
    {
        m_filePrefix = prefixJson.toString();
        m_prefixEdit->setText(m_filePrefix);
    }

    QJsonValue formatJson = model["formatIndex"];
    if (!formatJson.isUndefined())
    {
        m_formatIndex = formatJson.toInt();
        for (int i = 0; i < m_formatCombo->count(); ++i)
        {
            if (m_formatCombo->itemData(i).toInt() == m_formatIndex)
            {
                m_formatCombo->blockSignals(true);
                m_formatCombo->setCurrentIndex(i);
                m_formatCombo->blockSignals(false);
                break;
            }
        }
        onFormatChanged();
    }

    QJsonValue qualityJson = model["quality"];
    if (!qualityJson.isUndefined())
    {
        m_quality = qualityJson.toInt();
        m_qualitySpin->setValue(m_quality);
    }

    QJsonValue autoIncrementJson = model["autoIncrement"];
    if (!autoIncrementJson.isUndefined())
    {
        m_autoIncrement = autoIncrementJson.toBool();
        m_autoIncrementCheck->setChecked(m_autoIncrement);
    }

    QJsonValue frameCountJson = model["frameCount"];
    if (!frameCountJson.isUndefined())
    {
        m_frameCount = frameCountJson.toInt();
    }
}

} // namespace VisionBox
