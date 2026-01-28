/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * GrabCut Segmentation Model Implementation
 ******************************************************************************/

#include "GrabCutSegmentationModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <QCoreApplication>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
GrabCutSegmentationModel::GrabCutSegmentationModel()
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Initialization mode
    auto* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Init Mode:"));
    m_modeCombo = new QComboBox();
    m_modeCombo->addItem("Rectangle (Auto)", static_cast<int>(Rect));
    m_modeCombo->addItem("Mask Input", static_cast<int>(Mask));
    m_modeCombo->addItem("Auto Threshold", static_cast<int>(Auto));
    m_modeCombo->setCurrentIndex(0);
    m_modeCombo->setMinimumWidth(150);
    modeLayout->addWidget(m_modeCombo);
    layout->addLayout(modeLayout);

    // Iterations
    auto* iterLayout = new QHBoxLayout();
    iterLayout->addWidget(new QLabel("Iterations:"));
    m_iterationsSpin = new QSpinBox();
    m_iterationsSpin->setRange(1, 100);
    m_iterationsSpin->setValue(5);
    m_iterationsSpin->setSingleStep(1);
    iterLayout->addWidget(m_iterationsSpin);
    layout->addLayout(iterLayout);

    // Mask input option
    m_useMaskCheck = new QCheckBox("Use Mask Input (Port 2)");
    m_useMaskCheck->setToolTip("Use second input port as initialization mask");
    m_useMaskCheck->setChecked(false);
    layout->addWidget(m_useMaskCheck);

    // Show mask option
    m_showMaskCheck = new QCheckBox("Show Binary Mask");
    m_showMaskCheck->setToolTip("Output binary mask instead of segmented image");
    m_showMaskCheck->setChecked(false);
    layout->addWidget(m_showMaskCheck);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    m_runBtn = new QPushButton("Run Segmentation");
    m_resetBtn = new QPushButton("Reset");
    btnLayout->addWidget(m_runBtn);
    btnLayout->addWidget(m_resetBtn);
    layout->addLayout(btnLayout);

    // Status label
    m_statusLabel = new QLabel("Status: Ready");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; }");
    layout->addWidget(m_statusLabel);

    // Info text
    m_infoText = new QTextEdit();
    m_infoText->setReadOnly(true);
    m_infoText->setMaximumHeight(80);
    m_infoText->setPlaceholderText("Segmentation info will appear here...");
    layout->addWidget(m_infoText);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_iterationsSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GrabCutSegmentationModel::onIterationsChanged);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GrabCutSegmentationModel::onModeChanged);
    connect(m_useMaskCheck, &QCheckBox::stateChanged,
            this, &GrabCutSegmentationModel::onUseMaskChanged);
    connect(m_showMaskCheck, &QCheckBox::stateChanged,
            this, &GrabCutSegmentationModel::onShowMaskChanged);
    connect(m_runBtn, &QPushButton::clicked,
            this, &GrabCutSegmentationModel::onRunButtonClicked);
    connect(m_resetBtn, &QPushButton::clicked,
            this, &GrabCutSegmentationModel::onResetButtonClicked);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int GrabCutSegmentationModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return m_useMaskInput ? 2 : 1; // Image (and optional mask)
    }
    else
    {
        return 1; // One output image
    }
}

QtNodes::NodeDataType GrabCutSegmentationModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> GrabCutSegmentationModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return std::make_shared<ImageData>(m_outputImage);
}

void GrabCutSegmentationModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                        QtNodes::PortIndex portIndex)
{
    if (portIndex == 0)
    {
        m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

        // Reset if image changes
        if (m_inputImage && !m_inputImage->image().empty())
        {
            if (!m_initialized)
            {
                initializeMask();
            }
            else
            {
                // Apply existing mask
                cv::Mat image = m_inputImage->image();
                if (!m_mask.empty() && m_mask.size() == image.size())
                {
                    m_outputImage = applyMask(image, m_mask);
                }
                else
                {
                    m_outputImage = image.clone();
                }
            }
        }

        Q_EMIT dataUpdated(0);
    }
    else if (portIndex == 1 && m_useMaskInput)
    {
        m_maskImage = std::dynamic_pointer_cast<ImageData>(data);
    }
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* GrabCutSegmentationModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void GrabCutSegmentationModel::onIterationsChanged(int value)
{
    m_iterations = value;
}

void GrabCutSegmentationModel::onModeChanged(int index)
{
    m_initMode = m_modeCombo->itemData(index).toInt();
    m_initialized = false;
    m_statusLabel->setText("Status: Mode changed, re-initialization required");
}

void GrabCutSegmentationModel::onUseMaskChanged(int state)
{
    m_useMaskInput = (state == Qt::Checked);
    m_initialized = false;
    m_statusLabel->setText("Status: Port configuration changed");
}

void GrabCutSegmentationModel::onShowMaskChanged(int state)
{
    m_showMask = (state == Qt::Checked);

    // Update output if we have results
    if (m_inputImage && !m_mask.empty())
    {
        cv::Mat image = m_inputImage->image();
        if (m_showMask)
        {
            // Show binary mask
            cv::cvtColor(m_mask, m_outputImage, cv::COLOR_GRAY2BGR);
        }
        else
        {
            m_outputImage = applyMask(image, m_mask);
        }
        Q_EMIT dataUpdated(0);
    }
}

void GrabCutSegmentationModel::onRunButtonClicked()
{
    if (!m_inputImage || m_inputImage->image().empty())
    {
        m_statusLabel->setText("Status: No input image");
        return;
    }

    runSegmentation();
}

void GrabCutSegmentationModel::onResetButtonClicked()
{
    m_initialized = false;
    m_bgdModel.release();
    m_fgdModel.release();
    m_mask.release();
    m_statusLabel->setText("Status: Reset complete");

    if (m_inputImage && !m_inputImage->image().empty())
    {
        initializeMask();
    }
}

/*******************************************************************************
 * Segmentation
 ******************************************************************************/
void GrabCutSegmentationModel::runSegmentation()
{
    if (!m_inputImage)
    {
        return;
    }

    cv::Mat image = m_inputImage->image();
    if (image.empty())
    {
        m_statusLabel->setText("Status: Empty input image");
        return;
    }

    try
    {
        // Convert to 3-channel if needed
        cv::Mat image3c;
        if (image.channels() == 4)
        {
            cv::cvtColor(image, image3c, cv::COLOR_BGRA2BGR);
        }
        else if (image.channels() == 1)
        {
            cv::cvtColor(image, image3c, cv::COLOR_GRAY2BGR);
        }
        else
        {
            image3c = image;
        }

        // Initialize if needed
        if (!m_initialized || m_mask.empty() || m_mask.size() != image3c.size())
        {
            initializeMask();
        }

        // Run GrabCut algorithm
        if (m_mask.empty())
        {
            m_statusLabel->setText("Status: Initialization failed");
            return;
        }

        cv::grabCut(image3c, m_mask, m_rect, m_bgdModel, m_fgdModel,
                   m_iterations, cv::GC_INIT_WITH_RECT);

        // Extract foreground
        cv::Mat foregroundMask;
        cv::compare(m_mask, cv::GC_PR_FGD, foregroundMask, cv::CMP_EQ);
        // Also include definite foreground
        cv::Mat definiteFg;
        cv::compare(m_mask, cv::GC_FGD, definiteFg, cv::CMP_EQ);
        cv::bitwise_or(foregroundMask, definiteFg, foregroundMask);

        // Update mask for visualization
        m_mask = foregroundMask.clone();
        m_initialized = true;

        // Create output
        if (m_showMask)
        {
            cv::cvtColor(m_mask, m_outputImage, cv::COLOR_GRAY2BGR);
        }
        else
        {
            m_outputImage = applyMask(image3c, m_mask);
        }

        // Update info
        updateInfoText();

        m_statusLabel->setText(QString("Status: Segmentation complete (%1 iterations)")
                              .arg(m_iterations));

        Q_EMIT dataUpdated(0);
    }
    catch (const cv::Exception& e)
    {
        m_statusLabel->setText(QString("Status: Error - %1").arg(e.what()));
    }
}

void GrabCutSegmentationModel::initializeMask()
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

    try
    {
        int mode = m_modeCombo->currentData().toInt();

        if (mode == static_cast<int>(Mask) && m_maskImage)
        {
            // Use provided mask
            cv::Mat mask = m_maskImage->image();
            if (mask.channels() == 3)
            {
                cv::cvtColor(mask, mask, cv::COLOR_BGR2GRAY);
            }

            cv::threshold(mask, m_mask, 127, 255, cv::THRESH_BINARY);

            // Convert to GrabCut mask format
            m_mask.convertTo(m_mask, CV_8UC1);
            m_mask.setTo(cv::GC_PR_FGD, m_mask == 255);
            m_mask.setTo(cv::GC_BGD, m_mask == 0);

            m_rect = cv::Rect(0, 0, image.cols, image.rows);
        }
        else if (mode == static_cast<int>(Auto))
        {
            // Auto-initialize using Otsu thresholding
            cv::Mat gray;
            if (image.channels() == 3)
            {
                cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
            }
            else
            {
                gray = image.clone();
            }

            // Apply Otsu threshold
            cv::Mat binary;
            cv::threshold(gray, binary, 0, 255,
                         cv::THRESH_BINARY | cv::THRESH_OTSU);

            // Create GrabCut mask
            m_mask = cv::Mat::zeros(image.size(), CV_8UC1);
            m_mask.setTo(cv::GC_PR_BGD, binary == 0);
            m_mask.setTo(cv::GC_PR_FGD, binary == 255);

            m_rect = cv::Rect(0, 0, image.cols, image.rows);
        }
        else
        {
            // Rectangle mode - use centered rectangle
            int margin = std::min(image.cols, image.rows) / 10;
            m_rect = cv::Rect(
                margin,
                margin,
                image.cols - 2 * margin,
                image.rows - 2 * margin
            );

            m_mask = cv::Mat::zeros(image.size(), CV_8UC1);
            m_mask.setTo(cv::GC_BGD);
            m_mask(m_rect).setTo(cv::GC_PR_FGD);
        }

        m_initialized = false;
        m_statusLabel->setText("Status: Mask initialized, run segmentation");
    }
    catch (const cv::Exception& e)
    {
        m_statusLabel->setText(QString("Status: Init error - %1").arg(e.what()));
    }
}

cv::Mat GrabCutSegmentationModel::applyMask(const cv::Mat& image, const cv::Mat& mask)
{
    if (image.empty() || mask.empty())
    {
        return image.clone();
    }

    cv::Mat result = image.clone();

    // Create transparent background
    if (image.channels() == 3)
    {
        cv::Mat imageWithAlpha;
        cv::cvtColor(image, imageWithAlpha, cv::COLOR_BGR2BGRA);

        // Set background pixels to transparent
        for (int y = 0; y < mask.rows; ++y)
        {
            for (int x = 0; x < mask.cols; ++x)
            {
                if (mask.at<uchar>(y, x) == 0)
                {
                    cv::Vec4b& pixel = imageWithAlpha.at<cv::Vec4b>(y, x);
                    pixel[3] = 0;  // Set alpha to 0 (transparent)
                }
            }
        }

        return imageWithAlpha;
    }
    else
    {
        // For grayscale, just set background to black
        cv::Mat masked;
        image.copyTo(masked, mask);
        return masked;
    }
}

void GrabCutSegmentationModel::updateInfoText()
{
    if (m_mask.empty())
    {
        return;
    }

    // Count foreground pixels
    int fgPixels = cv::countNonZero(m_mask);
    int totalPixels = m_mask.rows * m_mask.cols;
    double fgPercent = (static_cast<double>(fgPixels) / totalPixels) * 100.0;

    QString info = QString("Foreground: %1 pixels (%.1f%%)\n")
                       .arg(fgPixels)
                       .arg(fgPercent);

    info += QString("Image size: %1x%2")
                .arg(m_mask.cols)
                .arg(m_mask.rows);

    m_infoText->setText(info);
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject GrabCutSegmentationModel::save() const
{
    QJsonObject modelJson;
    modelJson["iterations"] = m_iterations;
    modelJson["initMode"] = m_initMode;
    modelJson["useMaskInput"] = m_useMaskInput;
    modelJson["showMask"] = m_showMask;
    return modelJson;
}

void GrabCutSegmentationModel::load(QJsonObject const& model)
{
    QJsonValue iterJson = model["iterations"];
    if (!iterJson.isUndefined())
    {
        m_iterations = iterJson.toInt();
        m_iterationsSpin->setValue(m_iterations);
    }

    QJsonValue modeJson = model["initMode"];
    if (!modeJson.isUndefined())
    {
        m_initMode = modeJson.toInt();
        for (int i = 0; i < m_modeCombo->count(); ++i)
        {
            if (m_modeCombo->itemData(i).toInt() == m_initMode)
            {
                m_modeCombo->blockSignals(true);
                m_modeCombo->setCurrentIndex(i);
                m_modeCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue useMaskJson = model["useMaskInput"];
    if (!useMaskJson.isUndefined())
    {
        m_useMaskInput = useMaskJson.toBool();
        m_useMaskCheck->setChecked(m_useMaskInput);
    }

    QJsonValue showMaskJson = model["showMask"];
    if (!showMaskJson.isUndefined())
    {
        m_showMask = showMaskJson.toBool();
        m_showMaskCheck->setChecked(m_showMask);
    }

    m_initialized = false;
}

} // namespace VisionBox
