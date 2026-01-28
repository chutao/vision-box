/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Bounding Box Overlay Model Header
 ******************************************************************************/

#ifndef VISIONBOX_BOUNDINGBOXOVERLAYMODEL_H
#define VISIONBOX_BOUNDINGBOXOVERLAYMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QJsonArray>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * BoundingBoxOverlayModel - Draw bounding boxes and labels on images
 ******************************************************************************/
class BoundingBoxOverlayModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    BoundingBoxOverlayModel();
    ~BoundingBoxOverlayModel() override = default;

    QString caption() const override { return "Bounding Box Overlay"; }
    QString name() const override { return "BoundingBoxOverlayModel"; }

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
    void onBoxThicknessChanged(int value);
    void onBoxStyleChanged(int index);
    void onFontScaleChanged(double value);
    void onShowLabelsChanged(int state);
    void onShowConfidenceChanged(int state);
    void onColorModeChanged(int index);
    void onFixedColorChanged();

private:
    struct BoundingBox
    {
        int x;
        int y;
        int width;
        int height;
        int classId;
        float confidence;
        std::string label;
    };

    void processAndDraw();
    cv::Scalar getColorForClass(int classId);
    void updateInfoText();

private:
    // Drawing parameters
    int m_boxThickness = 2;              // Box line thickness
    int m_boxStyle = 0;                  // 0=Solid, 1=Dashed
    double m_fontScale = 0.5;            // Text size
    bool m_showLabels = true;            // Show class labels
    bool m_showConfidence = true;        // Show confidence scores
    int m_colorMode = 0;                 // 0=By Class, 1=Single Color, 2=Gradient

    // Color settings
    cv::Scalar m_fixedColor = cv::Scalar(0, 255, 0);  // Green (BGR)
    std::vector<cv::Scalar> m_classColors;

    // Detection results (from CSV/JSON input or manual)
    std::vector<BoundingBox> m_boxes;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_maskImage;
    cv::Mat m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QSpinBox* m_thicknessSpin = nullptr;
    QComboBox* m_boxStyleCombo = nullptr;
    QDoubleSpinBox* m_fontScaleSpin = nullptr;
    QCheckBox* m_showLabelsCheck = nullptr;
    QCheckBox* m_showConfidenceCheck = nullptr;
    QComboBox* m_colorModeCombo = nullptr;
    QPushButton* m_colorBtn = nullptr;
    QTextEdit* m_infoText = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_BOUNDINGBOXOVERLAYMODEL_H
