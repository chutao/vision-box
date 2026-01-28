/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Keypoint Viewer Model Implementation
 ******************************************************************************/

#include "KeypointViewerModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <QPushButton>
#include <QColorDialog>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
KeypointViewerModel::KeypointViewerModel()
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Point radius
    auto* radiusLayout = new QHBoxLayout();
    radiusLayout->addWidget(new QLabel("Point Radius:"));
    m_radiusSpin = new QSpinBox();
    m_radiusSpin->setRange(1, 20);
    m_radiusSpin->setValue(3);
    radiusLayout->addWidget(m_radiusSpin);
    layout->addLayout(radiusLayout);

    // Point style
    auto* styleLayout = new QHBoxLayout();
    styleLayout->addWidget(new QLabel("Point Style:"));
    m_styleCombo = new QComboBox();
    m_styleCombo->addItem("Circle", 0);
    m_styleCombo->addItem("Cross", 1);
    m_styleCombo->addItem("Plus", 2);
    m_styleCombo->addItem("Diamond", 3);
    m_styleCombo->setCurrentIndex(0);
    m_styleCombo->setMinimumWidth(150);
    styleLayout->addWidget(m_styleCombo);
    layout->addLayout(styleLayout);

    // Color mode
    auto* colorLayout = new QHBoxLayout();
    colorLayout->addWidget(new QLabel("Color Mode:"));
    m_colorModeCombo = new QComboBox();
    m_colorModeCombo->addItem("By Index", 0);
    m_colorModeCombo->addItem("Single Color", 1);
    m_colorModeCombo->addItem("Random", 2);
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

    // Visualization options
    m_drawCirclesCheck = new QCheckBox("Draw Filled Circles");
    m_drawCirclesCheck->setChecked(true);
    layout->addWidget(m_drawCirclesCheck);

    m_showConnectionsCheck = new QCheckBox("Show Connections");
    m_showConnectionsCheck->setToolTip("Draw lines between consecutive points");
    m_showConnectionsCheck->setChecked(false);
    layout->addWidget(m_showConnectionsCheck);

    // Info text
    m_infoText = new QTextEdit();
    m_infoText->setReadOnly(true);
    m_infoText->setMaximumHeight(80);
    m_infoText->setPlaceholderText("Keypoint info will appear here...");
    layout->addWidget(m_infoText);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_radiusSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &KeypointViewerModel::onPointRadiusChanged);
    connect(m_styleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KeypointViewerModel::onPointStyleChanged);
    connect(m_colorModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KeypointViewerModel::onColorModeChanged);
    connect(m_colorBtn, &QPushButton::clicked,
            this, &KeypointViewerModel::onFixedColorChanged);
    connect(m_drawCirclesCheck, &QCheckBox::stateChanged,
            this, &KeypointViewerModel::onDrawCirclesChanged);
    connect(m_showConnectionsCheck, &QCheckBox::stateChanged,
            this, &KeypointViewerModel::onShowConnectionsChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int KeypointViewerModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType KeypointViewerModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> KeypointViewerModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return std::make_shared<ImageData>(m_outputImage);
}

void KeypointViewerModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                    QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    if (m_inputImage && !m_inputImage->image().empty())
    {
        drawKeypoints();
    }

    Q_EMIT dataUpdated(0);
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* KeypointViewerModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void KeypointViewerModel::onPointRadiusChanged(int value)
{
    m_pointRadius = value;
    if (m_inputImage)
    {
        drawKeypoints();
        Q_EMIT dataUpdated(0);
    }
}

void KeypointViewerModel::onPointStyleChanged(int index)
{
    m_pointStyle = m_styleCombo->itemData(index).toInt();
    if (m_inputImage)
    {
        drawKeypoints();
        Q_EMIT dataUpdated(0);
    }
}

void KeypointViewerModel::onColorModeChanged(int index)
{
    m_colorMode = m_colorModeCombo->itemData(index).toInt();

    // Enable color button for single color mode
    m_colorBtn->setEnabled(m_colorMode == 1);

    if (m_inputImage)
    {
        drawKeypoints();
        Q_EMIT dataUpdated(0);
    }
}

void KeypointViewerModel::onDrawCirclesChanged(int state)
{
    m_drawCircles = (state == Qt::Checked);
    if (m_inputImage)
    {
        drawKeypoints();
        Q_EMIT dataUpdated(0);
    }
}

void KeypointViewerModel::onShowConnectionsChanged(int state)
{
    m_showConnections = (state == Qt::Checked);
    if (m_inputImage)
    {
        drawKeypoints();
        Q_EMIT dataUpdated(0);
    }
}

void KeypointViewerModel::onFixedColorChanged()
{
    QColor color = QColorDialog::getColor(
        QColor(m_fixedColor[2], m_fixedColor[1], m_fixedColor[0]),
        m_widget,
        "Select Point Color"
    );

    if (color.isValid())
    {
        m_fixedColor = cv::Scalar(color.blue(), color.green(), color.red());

        if (m_inputImage)
        {
            drawKeypoints();
            Q_EMIT dataUpdated(0);
        }
    }
}

/*******************************************************************************
 * Drawing
 ******************************************************************************/
void KeypointViewerModel::drawKeypoints()
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
    if (image.channels() == 1)
    {
        cv::cvtColor(image, m_outputImage, cv::COLOR_GRAY2BGR);
    }
    else
    {
        m_outputImage = image.clone();
    }

    // Draw connections if enabled
    if (m_showConnections && m_points.size() > 1)
    {
        for (size_t i = 1; i < m_points.size(); ++i)
        {
            cv::Scalar color = getColorForIndex(static_cast<int>(i));
            cv::line(m_outputImage, m_points[i-1], m_points[i], color, 1);
        }
    }

    // Draw keypoints/points
    for (size_t i = 0; i < m_points.size(); ++i)
    {
        cv::Point2f pt = m_points[i];
        cv::Scalar color = getColorForIndex(static_cast<int>(i));

        // Draw based on style
        switch (m_pointStyle)
        {
            case 0: // Circle
                if (m_drawCircles)
                {
                    cv::circle(m_outputImage,
                              cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)),
                              m_pointRadius, color, -1);
                }
                else
                {
                    cv::circle(m_outputImage,
                              cv::Point(static_cast<int>(pt.x), static_cast<int>(pt.y)),
                              m_pointRadius, color, 2);
                }
                break;

            case 1: // Cross
            {
                int r = m_pointRadius;
                cv::Point center(static_cast<int>(pt.x), static_cast<int>(pt.y));
                cv::line(m_outputImage, cv::Point(center.x - r, center.y),
                        cv::Point(center.x + r, center.y), color, 2);
                cv::line(m_outputImage, cv::Point(center.x, center.y - r),
                        cv::Point(center.x, center.y + r), color, 2);
                break;
            }

            case 2: // Plus
            {
                int r = m_pointRadius;
                cv::Point center(static_cast<int>(pt.x), static_cast<int>(pt.y));
                cv::line(m_outputImage, cv::Point(center.x - r, center.y - r),
                        cv::Point(center.x + r, center.y + r), color, 2);
                cv::line(m_outputImage, cv::Point(center.x + r, center.y - r),
                        cv::Point(center.x - r, center.y + r), color, 2);
                break;
            }

            case 3: // Diamond
            {
                int r = m_pointRadius;
                cv::Point center(static_cast<int>(pt.x), static_cast<int>(pt.y));
                std::vector<cv::Point> diamond = {
                    cv::Point(center.x, center.y - r),
                    cv::Point(center.x + r, center.y),
                    cv::Point(center.x, center.y + r),
                    cv::Point(center.x - r, center.y)
                };
                if (m_drawCircles)
                {
                    cv::fillConvexPoly(m_outputImage, diamond, color);
                }
                else
                {
                    cv::polylines(m_outputImage, diamond, true, color, 2);
                }
                break;
            }
        }
    }

    updateInfoText();
}

cv::Scalar KeypointViewerModel::getColorForIndex(int index)
{
    if (m_colorMode == 1)
    {
        // Single color
        return m_fixedColor;
    }
    else if (m_colorMode == 2)
    {
        // Random color (consistent per index)
        return cv::Scalar(
            (index * 73) % 256,
            (index * 137) % 256,
            (index * 191) % 256
        );
    }
    else
    {
        // By index (gradient)
        return cv::Scalar(
            (index * 37) % 256,
            (index * 97) % 256,
            (index * 151) % 256
        );
    }
}

void KeypointViewerModel::updateInfoText()
{
    QString info = QString("Points: %1\n").arg(m_points.size());
    info += QString("Image size: %1x%2")
                .arg(m_outputImage.cols)
                .arg(m_outputImage.rows);

    m_infoText->setText(info);
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject KeypointViewerModel::save() const
{
    QJsonObject modelJson;
    modelJson["pointRadius"] = m_pointRadius;
    modelJson["pointStyle"] = m_pointStyle;
    modelJson["colorMode"] = m_colorMode;
    modelJson["showConnections"] = m_showConnections;
    modelJson["drawCircles"] = m_drawCircles;

    QJsonArray colorArray;
    colorArray.append(m_fixedColor[0]);
    colorArray.append(m_fixedColor[1]);
    colorArray.append(m_fixedColor[2]);
    modelJson["fixedColor"] = colorArray;

    return modelJson;
}

void KeypointViewerModel::load(QJsonObject const& model)
{
    QJsonValue radiusJson = model["pointRadius"];
    if (!radiusJson.isUndefined())
    {
        m_pointRadius = radiusJson.toInt();
        // Validate radius to prevent OpenCV assertion failure
        if (m_pointRadius < 1) m_pointRadius = 1;
        if (m_pointRadius > 100) m_pointRadius = 100;
        m_radiusSpin->setValue(m_pointRadius);
    }

    QJsonValue styleJson = model["pointStyle"];
    if (!styleJson.isUndefined())
    {
        m_pointStyle = styleJson.toInt();
        for (int i = 0; i < m_styleCombo->count(); ++i)
        {
            if (m_styleCombo->itemData(i).toInt() == m_pointStyle)
            {
                m_styleCombo->blockSignals(true);
                m_styleCombo->setCurrentIndex(i);
                m_styleCombo->blockSignals(false);
                break;
            }
        }
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

    QJsonValue connectionsJson = model["showConnections"];
    if (!connectionsJson.isUndefined())
    {
        m_showConnections = connectionsJson.toBool();
        m_showConnectionsCheck->setChecked(m_showConnections);
    }

    QJsonValue circlesJson = model["drawCircles"];
    if (!circlesJson.isUndefined())
    {
        m_drawCircles = circlesJson.toBool();
        m_drawCirclesCheck->setChecked(m_drawCircles);
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
