/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Camera Source Node Model Implementation
 ******************************************************************************/

#include "CameraSourceModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <QTimer>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
CameraSourceModel::CameraSourceModel()
    : m_imageData(nullptr)
{
    // Create capture timer (30 FPS)
    m_captureTimer = new QTimer(this);
    m_captureTimer->setInterval(33);
    connect(m_captureTimer, &QTimer::timeout,
            this, &CameraSourceModel::captureFrame);

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Status label
    m_statusLabel = new QLabel("Status: Camera closed");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");

    // Camera selection
    auto* cameraLayout = new QHBoxLayout();
    cameraLayout->addWidget(new QLabel("Camera:"));

    m_cameraIdSpin = new QSpinBox();
    m_cameraIdSpin->setRange(0, 10);
    m_cameraIdSpin->setValue(0);
    m_cameraIdSpin->setMinimumWidth(60);
    cameraLayout->addWidget(m_cameraIdSpin);

    m_cameraCombo = new QComboBox();
    m_cameraCombo->addItem("Camera 0", 0);
    m_cameraCombo->addItem("Camera 1", 1);
    m_cameraCombo->addItem("Camera 2", 2);
    m_cameraCombo->addItem("Camera 3", 3);
    m_cameraCombo->setCurrentIndex(0);
    m_cameraCombo->setMinimumWidth(100);
    cameraLayout->addWidget(m_cameraCombo);

    // Resolution selection
    auto* resLayout = new QHBoxLayout();
    resLayout->addWidget(new QLabel("Resolution:"));

    m_resolutionCombo = new QComboBox();
    m_resolutionCombo->addItem("640x480", QSize(640, 480));
    m_resolutionCombo->addItem("1280x720", QSize(1280, 720));
    m_resolutionCombo->addItem("1920x1080", QSize(1920, 1080));
    m_resolutionCombo->addItem("320x240", QSize(320, 240));
    m_resolutionCombo->addItem("800x600", QSize(800, 600));
    m_resolutionCombo->addItem("1600x900", QSize(1600, 900));
    m_resolutionCombo->setCurrentIndex(0);
    m_resolutionCombo->setMinimumWidth(120);
    resLayout->addWidget(m_resolutionCombo);

    // Resolution info label
    m_resolutionLabel = new QLabel("Current: 640x480");
    m_resolutionLabel->setStyleSheet("QLabel { padding: 5px; }");

    // Buttons
    auto* buttonLayout = new QHBoxLayout();

    m_openButton = new QPushButton("Open Camera");
    m_openButton->setStyleSheet("QPushButton { padding: 5px; }");

    m_closeButton = new QPushButton("Close Camera");
    m_closeButton->setStyleSheet("QPushButton { padding: 5px; }");
    m_closeButton->setEnabled(false);

    buttonLayout->addWidget(m_openButton);
    buttonLayout->addWidget(m_closeButton);

    layout->addWidget(m_statusLabel);
    layout->addLayout(cameraLayout);
    layout->addLayout(resLayout);
    layout->addWidget(m_resolutionLabel);
    layout->addLayout(buttonLayout);
    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_openButton, &QPushButton::clicked,
            this, &CameraSourceModel::onOpenCamera);
    connect(m_closeButton, &QPushButton::clicked,
            this, &CameraSourceModel::onCloseCamera);
    connect(m_cameraIdSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CameraSourceModel::onCameraIdChanged);
    connect(m_cameraCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) { m_cameraIdSpin->setValue(m_cameraCombo->itemData(index).toInt()); });
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraSourceModel::onResolutionChanged);
}

CameraSourceModel::~CameraSourceModel()
{
    if (m_capture.isOpened())
    {
        m_capture.release();
    }
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int CameraSourceModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType CameraSourceModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> CameraSourceModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return m_imageData;
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* CameraSourceModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void CameraSourceModel::onOpenCamera()
{
    if (m_capture.isOpened())
    {
        m_capture.release();
    }

    m_cameraId = m_cameraIdSpin->value();

    // Open camera
    if (!m_capture.open(m_cameraId))
    {
        m_statusLabel->setText(QString("Status: Failed to open camera %1").arg(m_cameraId));
        m_isOpened = false;
        updateUI();
        return;
    }

    // Set resolution
    m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_width);
    m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_height);

    // Get actual resolution
    int actualWidth = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_WIDTH));
    int actualHeight = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_HEIGHT));

    m_width = actualWidth;
    m_height = actualHeight;
    m_isOpened = true;

    // Capture first frame
    captureFrame();

    // Start capture timer
    m_captureTimer->start();

    m_statusLabel->setText(QString("Status: Camera %1 open (%2)")
                           .arg(m_cameraId)
                           .arg(getResolutionString(m_width, m_height)));

    updateUI();
}

void CameraSourceModel::onCloseCamera()
{
    if (m_capture.isOpened())
    {
        m_capture.release();
    }

    m_captureTimer->stop();
    m_isOpened = false;
    m_imageData = nullptr;

    m_statusLabel->setText("Status: Camera closed");
    m_resolutionLabel->setText("Current: N/A");

    Q_EMIT dataUpdated(0);
    updateUI();
}

void CameraSourceModel::onCameraIdChanged(int id)
{
    m_cameraId = id;

    // Update combo box
    for (int i = 0; i < m_cameraCombo->count(); ++i)
    {
        if (m_cameraCombo->itemData(i).toInt() == id)
        {
            m_cameraCombo->blockSignals(true);
            m_cameraCombo->setCurrentIndex(i);
            m_cameraCombo->blockSignals(false);
            break;
        }
    }
}

void CameraSourceModel::onResolutionChanged()
{
    QSize size = m_resolutionCombo->currentData().toSize();
    m_width = size.width();
    m_height = size.height();

    // If camera is open, reconfigure it
    if (m_capture.isOpened())
    {
        bool wasTimerRunning = m_captureTimer->isActive();
        m_captureTimer->stop();

        m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_width);
        m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_height);

        // Get actual resolution
        int actualWidth = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_WIDTH));
        int actualHeight = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_HEIGHT));

        m_width = actualWidth;
        m_height = actualHeight;

        captureFrame();

        if (wasTimerRunning)
        {
            m_captureTimer->start();
        }

        m_statusLabel->setText(QString("Status: Camera %1 open (%2)")
                               .arg(m_cameraId)
                               .arg(getResolutionString(m_width, m_height)));
        m_resolutionLabel->setText(QString("Current: %1").arg(getResolutionString(m_width, m_height)));
    }
}

void CameraSourceModel::captureFrame()
{
    if (!m_capture.isOpened())
    {
        return;
    }

    cv::Mat frame;
    if (m_capture.read(frame) && !frame.empty())
    {
        m_imageData = std::make_shared<ImageData>(frame);
        Q_EMIT dataUpdated(0);
    }
}

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/
void CameraSourceModel::updateCameraList()
{
    // Could implement auto-detection here
    // For now, just use the manual combo box
}

void CameraSourceModel::updateUI()
{
    m_openButton->setEnabled(!m_isOpened);
    m_closeButton->setEnabled(m_isOpened);
    m_cameraIdSpin->setEnabled(!m_isOpened);
    m_cameraCombo->setEnabled(!m_isOpened);
    m_resolutionCombo->setEnabled(!m_isOpened);

    if (m_isOpened)
    {
        m_resolutionLabel->setText(QString("Current: %1").arg(getResolutionString(m_width, m_height)));
    }
}

QString CameraSourceModel::getResolutionString(int width, int height)
{
    return QString("%1x%2").arg(width).arg(height);
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject CameraSourceModel::save() const
{
    QJsonObject modelJson;
    modelJson["cameraId"] = m_cameraId;
    modelJson["width"] = m_width;
    modelJson["height"] = m_height;
    modelJson["isOpened"] = m_isOpened;
    return modelJson;
}

void CameraSourceModel::load(QJsonObject const& model)
{
    QJsonValue cameraIdJson = model["cameraId"];
    if (!cameraIdJson.isUndefined())
    {
        m_cameraId = cameraIdJson.toInt();
        m_cameraIdSpin->setValue(m_cameraId);
    }

    QJsonValue widthJson = model["width"];
    QJsonValue heightJson = model["height"];
    if (!widthJson.isUndefined() && !heightJson.isUndefined())
    {
        m_width = widthJson.toInt();
        m_height = heightJson.toInt();

        // Find matching resolution in combo
        for (int i = 0; i < m_resolutionCombo->count(); ++i)
        {
            QSize size = m_resolutionCombo->itemData(i).toSize();
            if (size.width() == m_width && size.height() == m_height)
            {
                m_resolutionCombo->blockSignals(true);
                m_resolutionCombo->setCurrentIndex(i);
                m_resolutionCombo->blockSignals(false);
                break;
            }
        }
    }

    // Auto-open camera if it was open before
    QJsonValue isOpenedJson = model["isOpened"];
    if (!isOpenedJson.isUndefined() && isOpenedJson.toBool())
    {
        onOpenCamera();
    }
}

} // namespace VisionBox
