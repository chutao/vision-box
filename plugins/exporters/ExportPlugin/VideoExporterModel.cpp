/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Video Exporter Model Implementation
 ******************************************************************************/

#include "VideoExporterModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <QFileInfo>
#include <QDir>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
VideoExporterModel::VideoExporterModel()
    : m_frameSize(0)
    , m_frameCount(0)
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Output file path
    auto* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(new QLabel("Output File:"));
    m_pathEdit = new QLineEdit();
    m_pathEdit->setPlaceholderText("/path/to/output.mp4");
    pathLayout->addWidget(m_pathEdit);
    m_browseBtn = new QPushButton("Browse...");
    pathLayout->addWidget(m_browseBtn);
    layout->addLayout(pathLayout);

    // Format selection
    auto* formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel("Format:"));
    m_formatCombo = new QComboBox();
    m_formatCombo->addItem("MP4 (H.264)", static_cast<int>(cv::VideoWriter::fourcc('m','p','4','v')));
    m_formatCombo->addItem("AVI (XVID)", static_cast<int>(cv::VideoWriter::fourcc('X','V','I','D')));
    m_formatCombo->addItem("AVI (MJPG)", static_cast<int>(cv::VideoWriter::fourcc('M','J','P','G')));
    m_formatCombo->addItem("AVI (DIVX)", static_cast<int>(cv::VideoWriter::fourcc('D','I','V','X')));
    m_formatCombo->addItem("MKV (H.264)", static_cast<int>(cv::VideoWriter::fourcc('m','p','4','v')));
    m_formatCombo->addItem("MOV (H.264)", static_cast<int>(cv::VideoWriter::fourcc('m','p','4','v')));
    m_formatCombo->setCurrentIndex(0);
    m_formatCombo->setMinimumWidth(150);
    formatLayout->addWidget(m_formatCombo);
    layout->addLayout(formatLayout);

    // FPS
    auto* fpsLayout = new QHBoxLayout();
    fpsLayout->addWidget(new QLabel("FPS:"));
    m_fpsSpin = new QDoubleSpinBox();
    m_fpsSpin->setRange(1.0, 120.0);
    m_fpsSpin->setSingleStep(1.0);
    m_fpsSpin->setValue(30.0);
    fpsLayout->addWidget(m_fpsSpin);
    layout->addLayout(fpsLayout);

    // Quality
    auto* qualityLayout = new QHBoxLayout();
    qualityLayout->addWidget(new QLabel("Quality:"));
    m_qualitySpin = new QSpinBox();
    m_qualitySpin->setRange(1, 100);
    m_qualitySpin->setValue(95);
    qualityLayout->addWidget(m_qualitySpin);
    layout->addLayout(qualityLayout);

    // Record button
    m_recordBtn = new QPushButton("Start Recording");
    m_recordBtn->setEnabled(false);
    layout->addWidget(m_recordBtn);

    // Status label
    m_statusLabel = new QLabel("Status: Idle");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");
    layout->addWidget(m_statusLabel);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_browseBtn, &QPushButton::clicked,
            this, &VideoExporterModel::onBrowseClicked);
    connect(m_recordBtn, &QPushButton::clicked,
            this, &VideoExporterModel::onToggleRecording);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VideoExporterModel::onFormatChanged);
    connect(m_fpsSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VideoExporterModel::onFpsChanged);
    connect(m_qualitySpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VideoExporterModel::onQualityChanged);
}

VideoExporterModel::~VideoExporterModel()
{
    if (m_writer.isOpened())
    {
        m_writer.release();
    }
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int VideoExporterModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image
    }
    else
    {
        return 0; // No output
    }
}

QtNodes::NodeDataType VideoExporterModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> VideoExporterModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return nullptr;
}

void VideoExporterModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                  QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    // Enable record button if we have data and path
    bool canRecord = m_inputImage != nullptr && !m_outputPath.isEmpty();
    m_recordBtn->setEnabled(canRecord);

    // Auto-write frame if recording
    if (m_state == Recording && m_inputImage)
    {
        cv::Mat image = m_inputImage->image();
        if (!image.empty())
        {
            writeFrame(image);
        }
    }
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* VideoExporterModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void VideoExporterModel::onBrowseClicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save Video",
        m_outputPath.isEmpty() ? (QDir::homePath() + "/output.mp4") : m_outputPath,
        "Video Files (*.mp4 *.avi *.mkv *.mov);;All Files (*.*)"
    );

    if (!filePath.isEmpty())
    {
        m_outputPath = filePath;
        m_pathEdit->setText(filePath);

        // Enable record button
        bool canRecord = m_inputImage != nullptr;
        m_recordBtn->setEnabled(canRecord);
    }
}

void VideoExporterModel::onToggleRecording()
{
    if (m_state == Idle || m_state == Paused)
    {
        // Start recording
        initializeWriter();
        if (m_writer.isOpened())
        {
            m_state = Recording;
            m_recordBtn->setText("Stop Recording");
            m_statusLabel->setText("Status: Recording...");
        }
    }
    else if (m_state == Recording)
    {
        // Stop recording
        finalizeWriter();
        m_state = Idle;
        m_recordBtn->setText("Start Recording");
        m_statusLabel->setText(QString("Status: Saved %1 frames").arg(m_frameCount));
        m_frameCount = 0;
    }
}

void VideoExporterModel::onFormatChanged()
{
    m_formatIndex = m_formatCombo->currentData().toInt();
}

void VideoExporterModel::onFpsChanged(double value)
{
    m_fps = value;
}

void VideoExporterModel::onQualityChanged(int value)
{
    m_quality = value;
}

/*******************************************************************************
 * Video Recording
 ******************************************************************************/
void VideoExporterModel::initializeWriter()
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

    // Get frame size from first image
    m_frameSize = static_cast<int>(image.total() * image.elemSize());

    // Get fourcc
    int fourcc = m_formatCombo->currentData().toInt();

    // Create output directory if needed
    QFileInfo fileInfo(m_outputPath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists())
    {
        dir.mkpath(dir.absolutePath());
    }

    // Initialize video writer
    try
    {
        m_writer.open(m_outputPath.toStdString(),
                     fourcc,
                     m_fps,
                     image.size(),
                     m_quality > 0);

        if (!m_writer.isOpened())
        {
            m_statusLabel->setText("Status: Failed to initialize writer");
            m_state = Idle;
            m_recordBtn->setText("Start Recording");
            return;
        }

        // Write first frame
        writeFrame(image);
    }
    catch (const cv::Exception& e)
    {
        m_statusLabel->setText(QString("Status: Error - %1").arg(e.what()));
        m_state = Idle;
        m_recordBtn->setText("Start Recording");
    }
}

void VideoExporterModel::finalizeWriter()
{
    if (m_writer.isOpened())
    {
        m_writer.release();
    }
}

void VideoExporterModel::writeFrame(const cv::Mat& frame)
{
    if (!m_writer.isOpened() || frame.empty())
    {
        return;
    }

    try
    {
        m_writer.write(frame);
        m_frameCount++;
        m_statusLabel->setText(QString("Status: Recording... (%1 frames)").arg(m_frameCount));
    }
    catch (const cv::Exception& e)
    {
        m_statusLabel->setText(QString("Status: Write error - %1").arg(e.what()));
        finalizeWriter();
        m_state = Idle;
        m_recordBtn->setText("Start Recording");
    }
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject VideoExporterModel::save() const
{
    QJsonObject modelJson;
    modelJson["outputPath"] = m_outputPath;
    modelJson["formatIndex"] = m_formatIndex;
    modelJson["fps"] = m_fps;
    modelJson["quality"] = m_quality;
    return modelJson;
}

void VideoExporterModel::load(QJsonObject const& model)
{
    QJsonValue pathJson = model["outputPath"];
    if (!pathJson.isUndefined())
    {
        m_outputPath = pathJson.toString();
        m_pathEdit->setText(m_outputPath);
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

    QJsonValue fpsJson = model["fps"];
    if (!fpsJson.isUndefined())
    {
        m_fps = fpsJson.toDouble();
        m_fpsSpin->setValue(m_fps);
    }

    QJsonValue qualityJson = model["quality"];
    if (!qualityJson.isUndefined())
    {
        m_quality = qualityJson.toInt();
        m_qualitySpin->setValue(m_quality);
    }
}

} // namespace VisionBox
