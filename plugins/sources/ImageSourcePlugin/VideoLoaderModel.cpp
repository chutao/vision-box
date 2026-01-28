/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Video Loader Node Model Implementation
 ******************************************************************************/

#include "VideoLoaderModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <QTimer>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
VideoLoaderModel::VideoLoaderModel()
    : m_imageData(nullptr)
{
    // Create playback timer
    m_playbackTimer = new QTimer(this);
    m_playbackTimer->setInterval(33); // ~30 FPS default
    connect(m_playbackTimer, &QTimer::timeout,
            this, &VideoLoaderModel::updateFrame);

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // File path label
    m_pathLabel = new QLabel("No video loaded");
    m_pathLabel->setWordWrap(true);
    m_pathLabel->setStyleSheet("QLabel { padding: 5px; }");

    // Browse button
    m_browseButton = new QPushButton("Browse...");
    m_browseButton->setStyleSheet("QPushButton { padding: 5px; }");

    // Frame info label
    m_frameLabel = new QLabel("Frame: 0 / 0");
    m_frameLabel->setStyleSheet("QLabel { padding: 5px; }");

    // Playback controls
    auto* controlLayout = new QHBoxLayout();

    m_playPauseButton = new QPushButton("Play");
    m_playPauseButton->setEnabled(false);
    m_playPauseButton->setStyleSheet("QPushButton { padding: 5px; }");

    m_frameSlider = new QSlider(Qt::Horizontal);
    m_frameSlider->setEnabled(false);
    m_frameSlider->setRange(0, 0);
    m_frameSlider->setValue(0);

    m_frameSpin = new QSpinBox();
    m_frameSpin->setEnabled(false);
    m_frameSpin->setRange(0, 0);
    m_frameSpin->setValue(0);
    m_frameSpin->setMinimumWidth(80);

    controlLayout->addWidget(m_playPauseButton);
    controlLayout->addWidget(m_frameSlider);
    controlLayout->addWidget(m_frameSpin);

    layout->addWidget(m_pathLabel);
    layout->addWidget(m_browseButton);
    layout->addWidget(m_frameLabel);
    layout->addLayout(controlLayout);
    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_browseButton, &QPushButton::clicked,
            this, &VideoLoaderModel::onBrowseClicked);
    connect(m_playPauseButton, &QPushButton::clicked,
            this, &VideoLoaderModel::onPlayPauseClicked);
    connect(m_frameSlider, &QSlider::valueChanged,
            this, &VideoLoaderModel::onFrameChanged);
    connect(m_frameSlider, &QSlider::sliderPressed,
            this, &VideoLoaderModel::onSliderPressed);
    connect(m_frameSlider, &QSlider::sliderReleased,
            this, &VideoLoaderModel::onSliderReleased);
    connect(m_frameSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VideoLoaderModel::onFrameChanged);
}

VideoLoaderModel::~VideoLoaderModel()
{
    if (m_capture.isOpened())
    {
        m_capture.release();
    }
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int VideoLoaderModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 0; // No input ports
    }
    else
    {
        return 1; // One output port for the current frame
    }
}

QtNodes::NodeDataType VideoLoaderModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> VideoLoaderModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_imageData;
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* VideoLoaderModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void VideoLoaderModel::onBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open Video",
        "",
        "Video Files (*.mp4 *.avi *.mov *.mkv *.wmv *.flv);;All Files (*.*)"
    );

    if (!filePath.isEmpty())
    {
        loadVideo(filePath);
    }
}

void VideoLoaderModel::onPlayPauseClicked()
{
    if (!m_capture.isOpened())
    {
        return;
    }

    m_isPlaying = !m_isPlaying;

    if (m_isPlaying)
    {
        m_playPauseButton->setText("Pause");
        m_playbackTimer->start();

        // Update timer interval based on FPS
        if (m_fps > 0)
        {
            int interval = static_cast<int>(1000.0 / m_fps);
            m_playbackTimer->setInterval(interval);
        }
    }
    else
    {
        m_playPauseButton->setText("Play");
        m_playbackTimer->stop();
    }
}

void VideoLoaderModel::onFrameChanged(int frame)
{
    if (m_isSeeking)
    {
        return;
    }

    seekToFrame(frame);
}

void VideoLoaderModel::onSliderPressed()
{
    m_isSeeking = true;
}

void VideoLoaderModel::onSliderReleased()
{
    m_isSeeking = false;
    int frame = m_frameSlider->value();
    seekToFrame(frame);
}

void VideoLoaderModel::updateFrame()
{
    if (!m_capture.isOpened() || m_isSeeking)
    {
        return;
    }

    cv::Mat frame;
    if (m_capture.read(frame))
    {
        m_currentFrame++;
        m_imageData = std::make_shared<ImageData>(frame);
        updateUI();
        Q_EMIT dataUpdated(0);
    }
    else
    {
        // End of video, stop playback
        m_isPlaying = false;
        m_playPauseButton->setText("Play");
        m_playbackTimer->stop();

        // Reset to beginning
        seekToFrame(0);
    }
}

/*******************************************************************************
 * Video Operations
 ******************************************************************************/
void VideoLoaderModel::loadVideo(const QString& filePath)
{
    // Close previous video
    if (m_capture.isOpened())
    {
        m_capture.release();
        m_playbackTimer->stop();
        m_isPlaying = false;
    }

    m_filePath = filePath;

    // Open video file
    if (!m_capture.open(filePath.toStdString()))
    {
        m_pathLabel->setText("Failed to load: " + filePath);
        m_playPauseButton->setEnabled(false);
        m_frameSlider->setEnabled(false);
        m_frameSpin->setEnabled(false);
        m_imageData = nullptr;
        m_totalFrames = 0;
        m_currentFrame = 0;
        updateUI();
        return;
    }

    // Get video properties
    m_totalFrames = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_COUNT));
    m_fps = m_capture.get(cv::CAP_PROP_FPS);
    m_currentFrame = 0;

    // Read first frame
    cv::Mat frame;
    if (m_capture.read(frame))
    {
        m_imageData = std::make_shared<ImageData>(frame);
        m_currentFrame = 1;
    }

    // Update UI
    m_pathLabel->setText("Loaded: " + QFileInfo(filePath).fileName());
    m_playPauseButton->setEnabled(true);
    m_playPauseButton->setText("Play");

    m_frameSlider->setEnabled(true);
    m_frameSlider->setRange(0, m_totalFrames - 1);
    m_frameSlider->setValue(0);

    m_frameSpin->setEnabled(true);
    m_frameSpin->setRange(0, m_totalFrames - 1);
    m_frameSpin->setValue(0);

    updateUI();
    Q_EMIT dataUpdated(0);
}

void VideoLoaderModel::seekToFrame(int frameNumber)
{
    if (!m_capture.isOpened())
    {
        return;
    }

    // Clamp frame number
    frameNumber = qBound(0, frameNumber, m_totalFrames - 1);

    // Set frame position
    m_capture.set(cv::CAP_PROP_POS_FRAMES, frameNumber);

    // Read the frame
    cv::Mat frame;
    if (m_capture.read(frame))
    {
        m_currentFrame = frameNumber + 1;
        m_imageData = std::make_shared<ImageData>(frame);
        updateUI();
        Q_EMIT dataUpdated(0);
    }
}

void VideoLoaderModel::updateUI()
{
    m_frameLabel->setText(QString("Frame: %1 / %2").arg(m_currentFrame).arg(m_totalFrames));

    // Block signals to prevent feedback loop
    m_frameSlider->blockSignals(true);
    m_frameSpin->blockSignals(true);

    m_frameSlider->setValue(m_currentFrame - 1);
    m_frameSpin->setValue(m_currentFrame - 1);

    m_frameSlider->blockSignals(false);
    m_frameSpin->blockSignals(false);
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject VideoLoaderModel::save() const
{
    QJsonObject modelJson;
    modelJson["filePath"] = m_filePath;
    modelJson["currentFrame"] = m_currentFrame;
    return modelJson;
}

void VideoLoaderModel::load(QJsonObject const& model)
{
    QJsonValue filePathJson = model["filePath"];
    if (!filePathJson.isUndefined())
    {
        QString filePath = filePathJson.toString();
        if (!filePath.isEmpty() && QFile::exists(filePath))
        {
            loadVideo(filePath);

            // Restore frame position
            QJsonValue frameJson = model["currentFrame"];
            if (!frameJson.isUndefined())
            {
                int frame = frameJson.toInt();
                if (frame > 0 && frame <= m_totalFrames)
                {
                    seekToFrame(frame - 1);
                }
            }
        }
    }
}

} // namespace VisionBox
