/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Bounding Box Overlay Model Implementation
 ******************************************************************************/

#include "BoundingBoxOverlayModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <QPushButton>
#include <QColorDialog>
#include <random>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
BoundingBoxOverlayModel::BoundingBoxOverlayModel()
{
    // Initialize class colors (COCO-style colors)
    m_classColors = {
        cv::Scalar(255, 0, 0),    // Red
        cv::Scalar(0, 255, 0),    // Green
        cv::Scalar(0, 0, 255),    // Blue
        cv::Scalar(255, 255, 0),  // Cyan
        cv::Scalar(255, 0, 255),  // Magenta
        cv::Scalar(0, 255, 255),  // Yellow
        cv::Scalar(128, 0, 128),  // Purple
        cv::Scalar(255, 165, 0),  // Orange
        cv::Scalar(255, 192, 203),// Pink
        cv::Scalar(0, 128, 128)   // Teal
    };

    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Box thickness
    auto* thicknessLayout = new QHBoxLayout();
    thicknessLayout->addWidget(new QLabel("Box Thickness:"));
    m_thicknessSpin = new QSpinBox();
    m_thicknessSpin->setRange(1, 10);
    m_thicknessSpin->setValue(2);
    thicknessLayout->addWidget(m_thicknessSpin);
    layout->addLayout(thicknessLayout);

    // Box style
    auto* styleLayout = new QHBoxLayout();
    styleLayout->addWidget(new QLabel("Box Style:"));
    m_boxStyleCombo = new QComboBox();
    m_boxStyleCombo->addItem("Solid", 0);
    m_boxStyleCombo->addItem("Dashed", 1);
    m_boxStyleCombo->setCurrentIndex(0);
    m_boxStyleCombo->setMinimumWidth(150);
    styleLayout->addWidget(m_boxStyleCombo);
    layout->addLayout(styleLayout);

    // Font scale
    auto* fontLayout = new QHBoxLayout();
    fontLayout->addWidget(new QLabel("Font Scale:"));
    m_fontScaleSpin = new QDoubleSpinBox();
    m_fontScaleSpin->setRange(0.1, 3.0);
    m_fontScaleSpin->setSingleStep(0.1);
    m_fontScaleSpin->setValue(0.5);
    fontLayout->addWidget(m_fontScaleSpin);
    layout->addLayout(fontLayout);

    // Visualization options
    m_showLabelsCheck = new QCheckBox("Show Labels");
    m_showLabelsCheck->setChecked(true);
    layout->addWidget(m_showLabelsCheck);

    m_showConfidenceCheck = new QCheckBox("Show Confidence");
    m_showConfidenceCheck->setChecked(true);
    layout->addWidget(m_showConfidenceCheck);

    // Color mode
    auto* colorLayout = new QHBoxLayout();
    colorLayout->addWidget(new QLabel("Color Mode:"));
    m_colorModeCombo = new QComboBox();
    m_colorModeCombo->addItem("By Class", 0);
    m_colorModeCombo->addItem("Single Color", 1);
    m_colorModeCombo->addItem("Gradient", 2);
    m_colorModeCombo->setCurrentIndex(0);
    m_colorModeCombo->setMinimumWidth(150);
    colorLayout->addWidget(m_colorModeCombo);
    layout->addLayout(colorLayout);

    // Color selection button
    auto* btnLayout = new QHBoxLayout();
    m_colorBtn = new QPushButton("Select Color");
    m_colorBtn->setEnabled(false);
    btnLayout->addWidget(m_colorBtn);
    layout->addLayout(btnLayout);

    // Info text
    m_infoText = new QTextEdit();
    m_infoText->setReadOnly(true);
    m_infoText->setMaximumHeight(80);
    m_infoText->setPlaceholderText("Bounding box info will appear here...");
    layout->addWidget(m_infoText);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_thicknessSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BoundingBoxOverlayModel::onBoxThicknessChanged);
    connect(m_boxStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundingBoxOverlayModel::onBoxStyleChanged);
    connect(m_fontScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &BoundingBoxOverlayModel::onFontScaleChanged);
    connect(m_showLabelsCheck, &QCheckBox::stateChanged,
            this, &BoundingBoxOverlayModel::onShowLabelsChanged);
    connect(m_showConfidenceCheck, &QCheckBox::stateChanged,
            this, &BoundingBoxOverlayModel::onShowConfidenceChanged);
    connect(m_colorModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BoundingBoxOverlayModel::onColorModeChanged);
    connect(m_colorBtn, &QPushButton::clicked,
            this, &BoundingBoxOverlayModel::onFixedColorChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int BoundingBoxOverlayModel::nPorts(QtNodes::PortType portType) const
{
    if (portType == QtNodes::PortType::In)
    {
        return 1; // One input image
    }
    else
    {
        return 1; // One output image
    }
}

QtNodes::NodeDataType BoundingBoxOverlayModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> BoundingBoxOverlayModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return std::make_shared<ImageData>(m_outputImage);
}

void BoundingBoxOverlayModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                       QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    if (m_inputImage && !m_inputImage->image().empty())
    {
        processAndDraw();
    }

    Q_EMIT dataUpdated(0);
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* BoundingBoxOverlayModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void BoundingBoxOverlayModel::onBoxThicknessChanged(int value)
{
    m_boxThickness = value;
    if (m_inputImage)
    {
        processAndDraw();
        Q_EMIT dataUpdated(0);
    }
}

void BoundingBoxOverlayModel::onBoxStyleChanged(int index)
{
    m_boxStyle = m_boxStyleCombo->itemData(index).toInt();
    if (m_inputImage)
    {
        processAndDraw();
        Q_EMIT dataUpdated(0);
    }
}

void BoundingBoxOverlayModel::onFontScaleChanged(double value)
{
    m_fontScale = value;
    if (m_inputImage)
    {
        processAndDraw();
        Q_EMIT dataUpdated(0);
    }
}

void BoundingBoxOverlayModel::onShowLabelsChanged(int state)
{
    m_showLabels = (state == Qt::Checked);
    if (m_inputImage)
    {
        processAndDraw();
        Q_EMIT dataUpdated(0);
    }
}

void BoundingBoxOverlayModel::onShowConfidenceChanged(int state)
{
    m_showConfidence = (state == Qt::Checked);
    if (m_inputImage)
    {
        processAndDraw();
        Q_EMIT dataUpdated(0);
    }
}

void BoundingBoxOverlayModel::onColorModeChanged(int index)
{
    m_colorMode = m_colorModeCombo->itemData(index).toInt();

    // Enable color button for single color mode
    m_colorBtn->setEnabled(m_colorMode == 1);

    if (m_inputImage)
    {
        processAndDraw();
        Q_EMIT dataUpdated(0);
    }
}

void BoundingBoxOverlayModel::onFixedColorChanged()
{
    QColor color = QColorDialog::getColor(
        QColor(m_fixedColor[2], m_fixedColor[1], m_fixedColor[0]),
        m_widget,
        "Select Box Color"
    );

    if (color.isValid())
    {
        m_fixedColor = cv::Scalar(color.blue(), color.green(), color.red());

        if (m_inputImage)
        {
            processAndDraw();
            Q_EMIT dataUpdated(0);
        }
    }
}

/*******************************************************************************
 * Processing and Drawing
 ******************************************************************************/
void BoundingBoxOverlayModel::processAndDraw()
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

    // Clone image for output
    if (image.channels() == 4)
    {
        cv::cvtColor(image, m_outputImage, cv::COLOR_BGRA2BGR);
    }
    else if (image.channels() == 1)
    {
        cv::cvtColor(image, m_outputImage, cv::COLOR_GRAY2BGR);
    }
    else
    {
        m_outputImage = image.clone();
    }

    // Draw all bounding boxes
    for (const auto& box : m_boxes)
    {
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::Scalar color = getColorForClass(box.classId);

        // Clip rectangle to image bounds
        rect.x = std::max(0, std::min(rect.x, m_outputImage.cols - 1));
        rect.y = std::max(0, std::min(rect.y, m_outputImage.rows - 1));
        rect.width = std::min(rect.width, m_outputImage.cols - rect.x);
        rect.height = std::min(rect.height, m_outputImage.rows - rect.y);

        if (rect.width <= 0 || rect.height <= 0)
        {
            continue;
        }

        // Draw box
        if (m_boxStyle == 0)
        {
            // Solid box
            cv::rectangle(m_outputImage, rect, color, m_boxThickness);
        }
        else
        {
            // Dashed box (simulated with lines)
            int dashLength = 10;
            cv::Point p1(rect.x, rect.y);
            cv::Point p2(rect.x + rect.width, rect.y);
            cv::Point p3(rect.x + rect.width, rect.y + rect.height);
            cv::Point p4(rect.x, rect.y + rect.height);

            // Draw dashed lines
            for (int i = 0; i < rect.width; i += dashLength * 2)
            {
                int len = std::min(dashLength, rect.width - i);
                cv::line(m_outputImage,
                        cv::Point(p1.x + i, p1.y),
                        cv::Point(p1.x + i + len, p1.y),
                        color, m_boxThickness);
            }
            for (int i = 0; i < rect.width; i += dashLength * 2)
            {
                int len = std::min(dashLength, rect.width - i);
                cv::line(m_outputImage,
                        cv::Point(p3.x - i, p3.y),
                        cv::Point(p3.x - i - len, p3.y),
                        color, m_boxThickness);
            }
            for (int i = 0; i < rect.height; i += dashLength * 2)
            {
                int len = std::min(dashLength, rect.height - i);
                cv::line(m_outputImage,
                        cv::Point(p1.x, p1.y + i),
                        cv::Point(p1.x, p1.y + i + len),
                        color, m_boxThickness);
            }
            for (int i = 0; i < rect.height; i += dashLength * 2)
            {
                int len = std::min(dashLength, rect.height - i);
                cv::line(m_outputImage,
                        cv::Point(p2.x, p2.y + i),
                        cv::Point(p2.x, p2.y + i + len),
                        color, m_boxThickness);
            }
        }

        // Draw label
        if (m_showLabels || m_showConfidence)
        {
            std::string label;
            if (m_showLabels && !box.label.empty())
            {
                label = box.label;
            }

            if (m_showConfidence && box.confidence > 0)
            {
                if (!label.empty())
                {
                    label += ": ";
                }
                label += std::to_string(static_cast<int>(box.confidence * 100)) + "%";
            }

            if (!label.empty())
            {
                int baseLine;
                cv::Size textSize =
                    cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                   m_fontScale, 1, &baseLine);

                // Draw label background
                cv::Rect labelRect(rect.x,
                                 rect.y - round(textSize.height) - baseLine,
                                 round(textSize.width),
                                 round(textSize.height) + baseLine);

                // Clip label rectangle
                if (labelRect.y < 0)
                {
                    labelRect.y = rect.y + rect.height;
                }

                cv::rectangle(m_outputImage, labelRect, color, cv::FILLED);

                // Draw label text
                cv::putText(m_outputImage, label,
                           cv::Point(rect.x, rect.y + textSize.height),
                           cv::FONT_HERSHEY_SIMPLEX, m_fontScale,
                           cv::Scalar(255, 255, 255), 1);
            }
        }
    }

    updateInfoText();
}

cv::Scalar BoundingBoxOverlayModel::getColorForClass(int classId)
{
    if (m_colorMode == 1)
    {
        // Single color
        return m_fixedColor;
    }
    else if (m_colorMode == 2)
    {
        // Gradient based on class ID
        return cv::Scalar(
            (classId * 37) % 256,
            (classId * 97) % 256,
            (classId * 151) % 256
        );
    }
    else
    {
        // By class (cycling through predefined colors)
        if (m_classColors.empty())
        {
            return cv::Scalar(0, 255, 0);
        }
        return m_classColors[classId % m_classColors.size()];
    }
}

void BoundingBoxOverlayModel::updateInfoText()
{
    QString info = QString("Boxes: %1\n").arg(m_boxes.size());

    if (!m_boxes.empty())
    {
        info += QString("Classes: ");
        std::set<int> uniqueClasses;
        for (const auto& box : m_boxes)
        {
            uniqueClasses.insert(box.classId);
        }
        info += QString::number(uniqueClasses.size()) + "\n";

        info += QString("Image size: %1x%2")
                    .arg(m_outputImage.cols)
                    .arg(m_outputImage.rows);
    }
    else
    {
        info += "No boxes to display\n";
        info += QString("Image size: %1x%2")
                    .arg(m_outputImage.cols)
                    .arg(m_outputImage.rows);
    }

    m_infoText->setText(info);
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject BoundingBoxOverlayModel::save() const
{
    QJsonObject modelJson;
    modelJson["boxThickness"] = m_boxThickness;
    modelJson["boxStyle"] = m_boxStyle;
    modelJson["fontScale"] = m_fontScale;
    modelJson["showLabels"] = m_showLabels;
    modelJson["showConfidence"] = m_showConfidence;
    modelJson["colorMode"] = m_colorMode;

    QJsonArray colorArray;
    colorArray.append(m_fixedColor[0]);
    colorArray.append(m_fixedColor[1]);
    colorArray.append(m_fixedColor[2]);
    modelJson["fixedColor"] = colorArray;

    return modelJson;
}

void BoundingBoxOverlayModel::load(QJsonObject const& model)
{
    QJsonValue thicknessJson = model["boxThickness"];
    if (!thicknessJson.isUndefined())
    {
        m_boxThickness = thicknessJson.toInt();
        m_thicknessSpin->setValue(m_boxThickness);
    }

    QJsonValue styleJson = model["boxStyle"];
    if (!styleJson.isUndefined())
    {
        m_boxStyle = styleJson.toInt();
        for (int i = 0; i < m_boxStyleCombo->count(); ++i)
        {
            if (m_boxStyleCombo->itemData(i).toInt() == m_boxStyle)
            {
                m_boxStyleCombo->blockSignals(true);
                m_boxStyleCombo->setCurrentIndex(i);
                m_boxStyleCombo->blockSignals(false);
                break;
            }
        }
    }

    QJsonValue fontJson = model["fontScale"];
    if (!fontJson.isUndefined())
    {
        m_fontScale = fontJson.toDouble();
        m_fontScaleSpin->setValue(m_fontScale);
    }

    QJsonValue labelsJson = model["showLabels"];
    if (!labelsJson.isUndefined())
    {
        m_showLabels = labelsJson.toBool();
        m_showLabelsCheck->setChecked(m_showLabels);
    }

    QJsonValue confJson = model["showConfidence"];
    if (!confJson.isUndefined())
    {
        m_showConfidence = confJson.toBool();
        m_showConfidenceCheck->setChecked(m_showConfidence);
    }

    QJsonValue colorModeJson = model["colorMode"];
    if (!colorModeJson.isUndefined())
    {
        m_colorMode = colorModeJson.toInt();
        for (int i = 0; i < m_colorModeCombo->count(); ++i)
        {
            if (m_colorModeCombo->itemData(i).toInt() == m_colorMode)
            {
                m_colorModeCombo->blockSignals(true);
                m_colorModeCombo->setCurrentIndex(i);
                m_colorModeCombo->blockSignals(false);
                break;
            }
        }
        m_colorBtn->setEnabled(m_colorMode == 1);
    }

    QJsonValue colorJson = model["fixedColor"];
    if (!colorJson.isUndefined() && colorJson.isArray())
    {
        QJsonArray colorArray = colorJson.toArray();
        if (colorArray.size() == 3)
        {
            m_fixedColor = cv::Scalar(
                colorArray[0].toDouble(),
                colorArray[1].toDouble(),
                colorArray[2].toDouble()
            );
        }
    }
}

} // namespace VisionBox
