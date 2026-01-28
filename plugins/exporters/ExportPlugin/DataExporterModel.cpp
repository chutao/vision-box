/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Data Exporter Model Implementation
 ******************************************************************************/

#include "DataExporterModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
DataExporterModel::DataExporterModel()
{
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
    m_prefixEdit->setPlaceholderText("data");
    prefixLayout->addWidget(m_prefixEdit);
    layout->addLayout(prefixLayout);

    // Format selection
    auto* formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel("Format:"));
    m_formatCombo = new QComboBox();
    m_formatCombo->addItem("CSV", static_cast<int>(CSV));
    m_formatCombo->addItem("JSON", static_cast<int>(JSON));
    m_formatCombo->setCurrentIndex(0);
    m_formatCombo->setMinimumWidth(150);
    formatLayout->addWidget(m_formatCombo);
    layout->addLayout(formatLayout);

    // Export type selection
    auto* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Data Type:"));
    m_exportTypeCombo = new QComboBox();
    m_exportTypeCombo->addItem("Image Info", static_cast<int>(ImageInfo));
    m_exportTypeCombo->addItem("Statistics", static_cast<int>(Statistics));
    m_exportTypeCombo->addItem("Histogram", static_cast<int>(Histogram));
    m_exportTypeCombo->addItem("Detection Results", static_cast<int>(DetectionResults));
    m_exportTypeCombo->setCurrentIndex(0);
    m_exportTypeCombo->setMinimumWidth(150);
    typeLayout->addWidget(m_exportTypeCombo);
    layout->addLayout(typeLayout);

    // Auto-increment
    m_autoIncrementCheck = new QCheckBox("Auto-Increment Filename");
    m_autoIncrementCheck->setToolTip("Add frame number to filename (e.g., data_001.csv)");
    layout->addWidget(m_autoIncrementCheck);

    // Include timestamp
    m_includeTimestampCheck = new QCheckBox("Include Timestamp");
    m_includeTimestampCheck->setToolTip("Add timestamp to filename (e.g., data_20250127_143000.csv)");
    layout->addWidget(m_includeTimestampCheck);

    // Export button
    m_exportBtn = new QPushButton("Export Data");
    m_exportBtn->setEnabled(false);
    layout->addWidget(m_exportBtn);

    // Status label
    m_statusLabel = new QLabel("Status: Ready");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");
    layout->addWidget(m_statusLabel);

    // Preview text
    m_previewText = new QTextEdit();
    m_previewText->setReadOnly(true);
    m_previewText->setMaximumHeight(100);
    m_previewText->setPlaceholderText("Data preview will appear here...");
    layout->addWidget(m_previewText);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_browseBtn, &QPushButton::clicked,
            this, &DataExporterModel::onBrowseClicked);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &DataExporterModel::onExportClicked);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataExporterModel::onFormatChanged);
    connect(m_exportTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataExporterModel::onExportTypeChanged);
    connect(m_autoIncrementCheck, &QCheckBox::stateChanged,
            this, &DataExporterModel::onAutoIncrementChanged);
    connect(m_includeTimestampCheck, &QCheckBox::stateChanged,
            this, &DataExporterModel::onIncludeTimestampChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int DataExporterModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image (optional)
    }
    else
    {
        return 0; // No output (sinks don't have outputs)
    }
}

QtNodes::NodeDataType DataExporterModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> DataExporterModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return nullptr;
}

void DataExporterModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                   QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    // Collect data from image when available
    if (m_inputImage)
    {
        collectDataFromImage();
        m_hasImageData = true;
    }
    else
    {
        m_hasImageData = false;
    }

    // Enable export button if we have data and path
    m_exportBtn->setEnabled(m_hasImageData && !m_outputPath.isEmpty());
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* DataExporterModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void DataExporterModel::onBrowseClicked()
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
        m_exportBtn->setEnabled(m_hasImageData);
    }
}

void DataExporterModel::onExportClicked()
{
    if (!m_hasImageData)
    {
        m_statusLabel->setText("Status: No data to export");
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

    // Export data
    if (exportData())
    {
        m_statusLabel->setText(QString("Status: Exported to %1").arg(generateFileName()));

        // Increment frame counter for next export
        if (m_autoIncrement)
        {
            m_frameCount++;
        }
    }
}

void DataExporterModel::onFormatChanged()
{
    m_formatIndex = m_formatCombo->currentData().toInt();
}

void DataExporterModel::onExportTypeChanged()
{
    m_exportTypeIndex = m_exportTypeCombo->currentData().toInt();

    // Update preview
    if (m_hasImageData)
    {
        collectDataFromImage();
    }
}

void DataExporterModel::onAutoIncrementChanged(int state)
{
    m_autoIncrement = (state == Qt::Checked);
    if (!m_autoIncrement)
    {
        m_frameCount = 0;
    }
}

void DataExporterModel::onIncludeTimestampChanged(int state)
{
    m_includeTimestamp = (state == Qt::Checked);
}

/*******************************************************************************
 * Data Export
 ******************************************************************************/
bool DataExporterModel::exportData()
{
    QString fileName = generateFileName();
    QString filePath = QDir(m_outputPath).absoluteFilePath(fileName);

    if (m_formatIndex == static_cast<int>(CSV))
    {
        return exportToCSV(filePath);
    }
    else if (m_formatIndex == static_cast<int>(JSON))
    {
        return exportToJSON(filePath);
    }

    return false;
}

bool DataExporterModel::exportToCSV(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_statusLabel->setText("Status: Failed to create file");
        return false;
    }

    QTextStream out(&file);

    try
    {
        int exportType = m_exportTypeCombo->currentData().toInt();

        if (exportType == static_cast<int>(ImageInfo) ||
            exportType == static_cast<int>(Statistics))
        {
            // Write header
            out << "Property,Value\n";

            // Write image info
            out << QString("Width,%1\n").arg(m_imageInfo.width);
            out << QString("Height,%1\n").arg(m_imageInfo.height);
            out << QString("Channels,%1\n").arg(m_imageInfo.channels);
            out << QString("Depth,%1\n").arg(m_imageInfo.depth);
            out << QString("Min Value,%1\n").arg(m_imageInfo.minValue, 0, 'f', 4);
            out << QString("Max Value,%1\n").arg(m_imageInfo.maxValue, 0, 'f', 4);

            if (exportType == static_cast<int>(Statistics))
            {
                out << QString("Mean,%1\n").arg(m_imageInfo.meanValue, 0, 'f', 6);
                out << QString("Std Dev,%1\n").arg(m_imageInfo.stdDev, 0, 'f', 6);
            }
        }
        else if (exportType == static_cast<int>(Histogram))
        {
            // Histogram data would be computed and exported here
            // For now, export basic stats
            out << "Property,Value\n";
            out << QString("Width,%1\n").arg(m_imageInfo.width);
            out << QString("Height,%1\n").arg(m_imageInfo.height);
            out << QString("Channels,%1\n").arg(m_imageInfo.channels);
            out << QString("Mean,%1\n").arg(m_imageInfo.meanValue, 0, 'f', 6);
        }
        else if (exportType == static_cast<int>(DetectionResults))
        {
            // Detection results would be exported here
            // Placeholder for future implementation
            out << "Frame,X,Y,Width,Height,Confidence,Class\n";
            out << "No detection data available\n";
        }

        file.close();
        return true;
    }
    catch (const std::exception& e)
    {
        file.close();
        m_statusLabel->setText(QString("Status: Export failed - %1").arg(e.what()));
        return false;
    }
}

bool DataExporterModel::exportToJSON(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_statusLabel->setText("Status: Failed to create file");
        return false;
    }

    try
    {
        QJsonObject root;
        int exportType = m_exportTypeCombo->currentData().toInt();

        if (exportType == static_cast<int>(ImageInfo) ||
            exportType == static_cast<int>(Statistics))
        {
            root["type"] = "image_info";
            root["timestamp"] = getCurrentTimestamp();

            QJsonObject info;
            info["width"] = m_imageInfo.width;
            info["height"] = m_imageInfo.height;
            info["channels"] = m_imageInfo.channels;
            info["depth"] = m_imageInfo.depth;
            info["min_value"] = m_imageInfo.minValue;
            info["max_value"] = m_imageInfo.maxValue;

            if (exportType == static_cast<int>(Statistics))
            {
                info["mean"] = m_imageInfo.meanValue;
                info["std_dev"] = m_imageInfo.stdDev;
            }

            root["data"] = info;
        }
        else if (exportType == static_cast<int>(Histogram))
        {
            root["type"] = "histogram";
            root["timestamp"] = getCurrentTimestamp();

            QJsonObject info;
            info["width"] = m_imageInfo.width;
            info["height"] = m_imageInfo.height;
            info["channels"] = m_imageInfo.channels;
            info["mean"] = m_imageInfo.meanValue;

            root["data"] = info;
        }
        else if (exportType == static_cast<int>(DetectionResults))
        {
            root["type"] = "detection_results";
            root["timestamp"] = getCurrentTimestamp();
            root["frame_number"] = m_frameCount;

            QJsonArray detections;
            // Placeholder for future detection data
            root["detections"] = detections;
        }

        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }
    catch (const std::exception& e)
    {
        file.close();
        m_statusLabel->setText(QString("Status: Export failed - %1").arg(e.what()));
        return false;
    }
}

QString DataExporterModel::generateFileName()
{
    QString prefix = m_prefixEdit->text().isEmpty() ? "data" : m_prefixEdit->text();
    QString extension = (m_formatIndex == static_cast<int>(CSV)) ? ".csv" : ".json";

    QString fileName = prefix;

    if (m_includeTimestamp)
    {
        fileName += "_" + getCurrentTimestamp();
    }

    if (m_autoIncrement)
    {
        // Format: prefix_001.ext or prefix_20250127_143000_001.ext
        fileName += QString("_%1").arg(m_frameCount, 3, 10, QChar('0'));
    }

    return fileName + extension;
}

QString DataExporterModel::getCurrentTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
}

void DataExporterModel::collectDataFromImage()
{
    if (!m_inputImage)
    {
        return;
    }

    cv::Mat image = m_inputImage->image();
    if (image.empty())
    {
        return;
    }

    // Basic image info
    m_imageInfo.width = image.cols;
    m_imageInfo.height = image.rows;
    m_imageInfo.channels = image.channels();
    m_imageInfo.depth = image.depth();

    // Compute min/max
    cv::Mat mask;
    double minVal, maxVal;
    cv::minMaxLoc(image, &minVal, &maxVal, nullptr, nullptr, mask);
    m_imageInfo.minValue = minVal;
    m_imageInfo.maxValue = maxVal;

    // Compute mean and standard deviation
    cv::Mat mean, stddev;
    cv::meanStdDev(image, mean, stddev);
    m_imageInfo.meanValue = mean.at<double>(0);
    m_imageInfo.stdDev = stddev.at<double>(0);

    // Update preview
    QString preview = QString("Width: %1, Height: %2, Channels: %3\n")
                          .arg(m_imageInfo.width)
                          .arg(m_imageInfo.height)
                          .arg(m_imageInfo.channels);
    preview += QString("Range: [%1, %2]\n")
                    .arg(m_imageInfo.minValue, 0, 'f', 2)
                    .arg(m_imageInfo.maxValue, 0, 'f', 2);
    preview += QString("Mean: %1, StdDev: %2")
                    .arg(m_imageInfo.meanValue, 0, 'f', 4)
                    .arg(m_imageInfo.stdDev, 0, 'f', 4);

    m_previewText->setText(preview);
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject DataExporterModel::save() const
{
    QJsonObject modelJson;
    modelJson["outputPath"] = m_outputPath;
    modelJson["filePrefix"] = m_filePrefix;
    modelJson["formatIndex"] = m_formatIndex;
    modelJson["exportTypeIndex"] = m_exportTypeIndex;
    modelJson["autoIncrement"] = m_autoIncrement;
    modelJson["includeTimestamp"] = m_includeTimestamp;
    modelJson["frameCount"] = m_frameCount;
    return modelJson;
}

void DataExporterModel::load(QJsonObject const& model)
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
    }

    QJsonValue exportTypeJson = model["exportTypeIndex"];
    if (!exportTypeJson.isUndefined())
    {
        m_exportTypeIndex = exportTypeJson.toInt();
        for (int i = 0; i < m_exportTypeCombo->count(); ++i)
        {
            if (m_exportTypeCombo->itemData(i).toInt() == m_exportTypeIndex)
            {
                m_exportTypeCombo->blockSignals(true);
                m_exportTypeCombo->setCurrentIndex(i);
                m_exportTypeCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue autoIncrementJson = model["autoIncrement"];
    if (!autoIncrementJson.isUndefined())
    {
        m_autoIncrement = autoIncrementJson.toBool();
        m_autoIncrementCheck->setChecked(m_autoIncrement);
    }

    QJsonValue includeTimestampJson = model["includeTimestamp"];
    if (!includeTimestampJson.isUndefined())
    {
        m_includeTimestamp = includeTimestampJson.toBool();
        m_includeTimestampCheck->setChecked(m_includeTimestamp);
    }

    QJsonValue frameCountJson = model["frameCount"];
    if (!frameCountJson.isUndefined())
    {
        m_frameCount = frameCountJson.toInt();
    }
}

} // namespace VisionBox
