/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Drawing Overlay Model Implementation
 ******************************************************************************/

#include "DrawingOverlayModel.h"
#include "core/VisionDataTypes.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <QPushButton>
#include <QColorDialog>

namespace VisionBox {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
DrawingOverlayModel::DrawingOverlayModel()
{
    // Create embedded widget
    m_widget = new QWidget();
    auto* layout = new QVBoxLayout(m_widget);

    // Shape type
    auto* shapeLayout = new QHBoxLayout();
    shapeLayout->addWidget(new QLabel("Shape:"));
    m_shapeCombo = new QComboBox();
    m_shapeCombo->addItem("Rectangle", static_cast<int>(Rectangle));
    m_shapeCombo->addItem("Circle", static_cast<int>(Circle));
    m_shapeCombo->addItem("Line", static_cast<int>(Line));
    m_shapeCombo->addItem("Arrow", static_cast<int>(Arrow));
    m_shapeCombo->addItem("Text", static_cast<int>(Text));
    m_shapeCombo->setCurrentIndex(0);
    m_shapeCombo->setMinimumWidth(150);
    shapeLayout->addWidget(m_shapeCombo);
    layout->addLayout(shapeLayout);

    // Thickness
    auto* thicknessLayout = new QHBoxLayout();
    thicknessLayout->addWidget(new QLabel("Thickness:"));
    m_thicknessSpin = new QSpinBox();
    m_thicknessSpin->setRange(1, 20);
    m_thicknessSpin->setValue(2);
    thicknessLayout->addWidget(m_thicknessSpin);
    layout->addLayout(thicknessLayout);

    // Size (for rectangle, circle)
    auto* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Size:"));
    m_sizeSpin = new QSpinBox();
    m_sizeSpin->setRange(10, 500);
    m_sizeSpin->setValue(50);
    sizeLayout->addWidget(m_sizeSpin);
    layout->addLayout(sizeLayout);

    // Position X
    auto* posXLayout = new QHBoxLayout();
    posXLayout->addWidget(new QLabel("Pos X:"));
    m_posXSlider = new QSlider(Qt::Horizontal);
    m_posXSlider->setRange(0, 1920);
    m_posXSlider->setValue(100);
    posXLayout->addWidget(m_posXSlider);
    layout->addLayout(posXLayout);

    // Position Y
    auto* posYLayout = new QHBoxLayout();
    posYLayout->addWidget(new QLabel("Pos Y:"));
    m_posYSlider = new QSlider(Qt::Horizontal);
    m_posYSlider->setRange(0, 1080);
    m_posYSlider->setValue(100);
    posYLayout->addWidget(m_posYSlider);
    layout->addLayout(posYLayout);

    // Text (for text shape)
    auto* textLayout = new QHBoxLayout();
    textLayout->addWidget(new QLabel("Text:"));
    m_textEdit = new QLineEdit();
    m_textEdit->setText("Sample Text");
    textLayout->addWidget(m_textEdit);
    layout->addLayout(textLayout);

    // Font size
    auto* fontLayout = new QHBoxLayout();
    fontLayout->addWidget(new QLabel("Font Size:"));
    m_fontSizeSpin = new QSpinBox();
    m_fontSizeSpin->setRange(10, 100);
    m_fontSizeSpin->setValue(30);
    fontLayout->addWidget(m_fontSizeSpin);
    layout->addLayout(fontLayout);

    // Color button
    auto* btnLayout = new QHBoxLayout();
    m_colorBtn = new QPushButton("Select Color");
    btnLayout->addWidget(m_colorBtn);
    layout->addLayout(btnLayout);

    // Draw checkbox
    m_drawShapeCheck = new QCheckBox("Draw Shape");
    m_drawShapeCheck->setChecked(true);
    layout->addWidget(m_drawShapeCheck);

    // Info text
    m_infoText = new QTextEdit();
    m_infoText->setReadOnly(true);
    m_infoText->setMaximumHeight(60);
    m_infoText->setPlaceholderText("Shape info will appear here...");
    layout->addWidget(m_infoText);

    layout->setContentsMargins(5, 5, 5, 5);

    // Connect signals
    connect(m_shapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DrawingOverlayModel::onShapeTypeChanged);
    connect(m_thicknessSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DrawingOverlayModel::onThicknessChanged);
    connect(m_colorBtn, &QPushButton::clicked,
            this, &DrawingOverlayModel::onColorChanged);
    connect(m_textEdit, &QLineEdit::textChanged,
            this, &DrawingOverlayModel::onTextChanged);
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DrawingOverlayModel::onFontSizeChanged);
    connect(m_posXSlider, &QSlider::valueChanged,
            this, &DrawingOverlayModel::onPositionChanged);
    connect(m_posYSlider, &QSlider::valueChanged,
            this, &DrawingOverlayModel::onPositionChanged);
    connect(m_sizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DrawingOverlayModel::onPositionChanged);
    connect(m_drawShapeCheck, &QCheckBox::stateChanged,
            this, &DrawingOverlayModel::onDrawShapeChanged);
}

/*******************************************************************************
 * Port Configuration
 ******************************************************************************/
unsigned int DrawingOverlayModel::nPorts(QtNodes::PortType portType) const
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

QtNodes::NodeDataType DrawingOverlayModel::dataType(
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
std::shared_ptr<QtNodes::NodeData> DrawingOverlayModel::outData(QtNodes::PortIndex port)
{
    Q_UNUSED(port);
    return std::make_shared<ImageData>(m_outputImage);
}

void DrawingOverlayModel::setInData(std::shared_ptr<QtNodes::NodeData> data,
                                    QtNodes::PortIndex portIndex)
{
    Q_UNUSED(portIndex);
    m_inputImage = std::dynamic_pointer_cast<ImageData>(data);

    if (m_inputImage && !m_inputImage->image().empty())
    {
        // Update slider ranges based on image size
        cv::Mat image = m_inputImage->image();
        m_posXSlider->setRange(0, image.cols);
        m_posYSlider->setRange(0, image.rows);

        drawOverlay();
    }

    Q_EMIT dataUpdated(0);
}

/*******************************************************************************
 * Widget
 ******************************************************************************/
QWidget* DrawingOverlayModel::embeddedWidget()
{
    return m_widget;
}

/*******************************************************************************
 * Slots
 ******************************************************************************/
void DrawingOverlayModel::onShapeTypeChanged(int index)
{
    m_shapeType = m_shapeCombo->itemData(index).toInt();

    // Update UI based on shape type
    bool isText = (m_shapeType == static_cast<int>(Text));
    m_textEdit->setEnabled(isText);
    m_fontSizeSpin->setEnabled(isText);

    if (m_inputImage)
    {
        drawOverlay();
        Q_EMIT dataUpdated(0);
    }
}

void DrawingOverlayModel::onThicknessChanged(int value)
{
    m_thickness = value;
    if (m_inputImage)
    {
        drawOverlay();
        Q_EMIT dataUpdated(0);
    }
}

void DrawingOverlayModel::onColorChanged()
{
    QColor color = QColorDialog::getColor(
        QColor(m_color[2], m_color[1], m_color[0]),
        m_widget,
        "Select Drawing Color"
    );

    if (color.isValid())
    {
        m_color = cv::Scalar(color.blue(), color.green(), color.red());

        if (m_inputImage)
        {
            drawOverlay();
            Q_EMIT dataUpdated(0);
        }
    }
}

void DrawingOverlayModel::onTextChanged()
{
    m_text = m_textEdit->text();
    if (m_inputImage)
    {
        drawOverlay();
        Q_EMIT dataUpdated(0);
    }
}

void DrawingOverlayModel::onFontSizeChanged(int value)
{
    m_fontScale = value / 30.0;  // Normalize to 1.0 base
    if (m_inputImage)
    {
        drawOverlay();
        Q_EMIT dataUpdated(0);
    }
}

void DrawingOverlayModel::onPositionChanged()
{
    m_positionX = m_posXSlider->value();
    m_positionY = m_posYSlider->value();
    m_size = m_sizeSpin->value();

    if (m_inputImage)
    {
        drawOverlay();
        Q_EMIT dataUpdated(0);
    }
}

void DrawingOverlayModel::onDrawShapeChanged(int state)
{
    m_drawShape = (state == Qt::Checked);

    if (m_inputImage)
    {
        drawOverlay();
        Q_EMIT dataUpdated(0);
    }
}

/*******************************************************************************
 * Drawing
 ******************************************************************************/
void DrawingOverlayModel::drawOverlay()
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
    else if (image.channels() == 4)
    {
        cv::cvtColor(image, m_outputImage, cv::COLOR_BGRA2BGR);
    }
    else
    {
        m_outputImage = image.clone();
    }

    if (!m_drawShape)
    {
        return;
    }

    cv::Point pt1(m_positionX, m_positionY);
    cv::Point pt2(m_positionX + m_size, m_positionY + m_size);
    cv::Scalar color = getDrawColor();

    switch (m_shapeType)
    {
        case Rectangle:
            cv::rectangle(m_outputImage, pt1, pt2, color, m_thickness);
            break;

        case Circle:
            cv::circle(m_outputImage, pt1, m_size / 2, color, m_thickness);
            break;

        case Line:
            cv::line(m_outputImage, pt1, pt2, color, m_thickness);
            break;

        case Arrow:
            cv::arrowedLine(m_outputImage, pt1, pt2, color, m_thickness);
            break;

        case Text:
            if (!m_text.isEmpty())
            {
                std::string textStr = m_text.toStdString();
                int baseline = 0;
                cv::Size textSize =
                    cv::getTextSize(textStr, cv::FONT_HERSHEY_SIMPLEX,
                                   m_fontScale, m_thickness, &baseline);

                // Draw text background
                cv::Rect textRect(pt1.x, pt1.y - textSize.height,
                                textSize.width, textSize.height + baseline);
                cv::rectangle(m_outputImage, textRect, color, cv::FILLED);

                // Draw text
                cv::putText(m_outputImage, textStr,
                           cv::Point(pt1.x, pt1.y),
                           cv::FONT_HERSHEY_SIMPLEX, m_fontScale,
                           cv::Scalar(255, 255, 255), m_thickness);
            }
            break;
    }

    // Update info
    QString info = QString("Shape: %1\nPos: (%2, %3)\nSize: %4")
                       .arg(m_shapeCombo->currentText())
                       .arg(m_positionX)
                       .arg(m_positionY)
                       .arg(m_size);
    m_infoText->setText(info);
}

cv::Scalar DrawingOverlayModel::getDrawColor()
{
    return m_color;
}

/*******************************************************************************
 * Serialization
 ******************************************************************************/
QJsonObject DrawingOverlayModel::save() const
{
    QJsonObject modelJson;
    modelJson["shapeType"] = m_shapeType;
    modelJson["thickness"] = m_thickness;
    modelJson["text"] = m_text;
    modelJson["fontScale"] = m_fontScale;
    modelJson["positionX"] = m_positionX;
    modelJson["positionY"] = m_positionY;
    modelJson["size"] = m_size;
    modelJson["drawShape"] = m_drawShape;

    QJsonArray colorArray;
    colorArray.append(m_color[0]);
    colorArray.append(m_color[1]);
    colorArray.append(m_color[2]);
    modelJson["color"] = colorArray;

    return modelJson;
}

void DrawingOverlayModel::load(QJsonObject const& model)
{
    QJsonValue shapeJson = model["shapeType"];
    if (!shapeJson.isUndefined())
    {
        m_shapeType = shapeJson.toInt();
        for (int i = 0; i < m_shapeCombo->count(); ++i)
        {
            if (m_shapeCombo->itemData(i).toInt() == m_shapeType)
            {
                m_shapeCombo->blockSignals(true);
                m_shapeCombo->setCurrentIndex(i);
                m_shapeCombo->blockSignals(false);
                break;
            }
        }

        bool isText = (m_shapeType == static_cast<int>(Text));
        m_textEdit->setEnabled(isText);
        m_fontSizeSpin->setEnabled(isText);
    }

    QJsonValue thicknessJson = model["thickness"];
    if (!thicknessJson.isUndefined())
    {
        m_thickness = thicknessJson.toInt();
        // Validate thickness (OpenCV requires thickness >= -1 and <= MAX_THICKNESS)
        if (m_thickness < -1) m_thickness = -1;  // -1 means filled
        if (m_thickness > 20) m_thickness = 20;
        m_thicknessSpin->setValue(m_thickness);
    }

    QJsonValue textJson = model["text"];
    if (!textJson.isUndefined())
    {
        m_text = textJson.toString();
        m_textEdit->setText(m_text);
    }

    QJsonValue fontJson = model["fontScale"];
    if (!fontJson.isUndefined())
    {
        m_fontScale = fontJson.toDouble();
        // Validate font scale
        if (m_fontScale < 0.1) m_fontScale = 0.1;
        if (m_fontScale > 10.0) m_fontScale = 10.0;
        m_fontSizeSpin->setValue(static_cast<int>(m_fontScale * 30));
    }

    QJsonValue posXJson = model["positionX"];
    if (!posXJson.isUndefined())
    {
        m_positionX = posXJson.toInt();
        // Validate position
        if (m_positionX < 0) m_positionX = 0;
        m_posXSlider->setValue(m_positionX);
    }

    QJsonValue sizeJson = model["size"];
    if (!sizeJson.isUndefined())
    {
        m_size = sizeJson.toInt();
        // Validate size to prevent division by zero or negative radius in circle
        if (m_size < 2) m_size = 2;  // Minimum size to ensure radius >= 1
        if (m_size > 1000) m_size = 1000;
        m_sizeSpin->setValue(m_size);
    }

    QJsonValue drawJson = model["drawShape"];
    if (!drawJson.isUndefined())
    {
        m_drawShape = drawJson.toBool();
        m_drawShapeCheck->setChecked(m_drawShape);
    }

    QJsonValue colorJson = model["color"];
    if (!colorJson.isUndefined() && colorJson.isArray())
    {
        QJsonArray colorArray = colorJson.toArray();
        if (colorArray.size() == 3)
        {
            m_color = cv::Scalar(
                colorArray[0].toDouble(),
                colorArray[1].toDouble(),
                colorArray[2].toDouble()
            );
        }
    }
}

} // namespace VisionBox
