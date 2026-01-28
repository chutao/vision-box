/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Drawing Overlay Model Header
 ******************************************************************************/

#ifndef VISIONBOX_DRAWINGOVERLAYMODEL_H
#define VISIONBOX_DRAWINGOVERLAYMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QSlider>
#include <QJsonArray>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * DrawingOverlayModel - Draw shapes and text on images
 ******************************************************************************/
class DrawingOverlayModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    DrawingOverlayModel();
    ~DrawingOverlayModel() override = default;

    QString caption() const override { return "Drawing Overlay"; }
    QString name() const override { return "DrawingOverlayModel"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;

    QWidget* embeddedWidget() override;

    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

private slots:
    void onShapeTypeChanged(int index);
    void onThicknessChanged(int value);
    void onColorChanged();
    void onTextChanged();
    void onFontSizeChanged(int value);
    void onPositionChanged();
    void onDrawShapeChanged(int state);

private:
    enum ShapeType
    {
        Rectangle,
        Circle,
        Line,
        Arrow,
        Text
    };

    void drawOverlay();
    cv::Scalar getDrawColor();

private:
    // Drawing parameters
    int m_shapeType = Rectangle;
    int m_thickness = 2;
    cv::Scalar m_color = cv::Scalar(255, 0, 0);  // Blue (BGR)
    QString m_text = "Sample Text";
    double m_fontScale = 1.0;
    int m_positionX = 100;
    int m_positionY = 100;
    int m_size = 50;
    bool m_drawShape = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    cv::Mat m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_shapeCombo = nullptr;
    QSpinBox* m_thicknessSpin = nullptr;
    QPushButton* m_colorBtn = nullptr;
    QLineEdit* m_textEdit = nullptr;
    QSpinBox* m_fontSizeSpin = nullptr;
    QSlider* m_posXSlider = nullptr;
    QSlider* m_posYSlider = nullptr;
    QSpinBox* m_sizeSpin = nullptr;
    QCheckBox* m_drawShapeCheck = nullptr;
    QTextEdit* m_infoText = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_DRAWINGOVERLAYMODEL_H
