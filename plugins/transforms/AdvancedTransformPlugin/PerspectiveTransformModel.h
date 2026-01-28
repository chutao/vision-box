/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Perspective Transform Model Header
 ******************************************************************************/

#ifndef VISIONBOX_PERSPECTIVETRANSFORMMODEL_H
#define VISIONBOX_PERSPECTIVETRANSFORMMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * PerspectiveTransformModel - Apply perspective transformation to images
 ******************************************************************************/
class PerspectiveTransformModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    PerspectiveTransformModel();
    ~PerspectiveTransformModel() override = default;

    QString caption() const override { return "Perspective Transform"; }
    QString name() const override { return "PerspectiveTransformModel"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;

    QWidget* embeddedWidget() override;

    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

public slots:
    void onPointChanged();
    void onOutputWidthChanged(int value);
    void onOutputHeightChanged(int value);
    void onInterpolationChanged();
    void onBorderModeChanged();
    void onBorderValueChanged(double value);
    void onAutoRectChanged(int state);

private:
    void applyTransform();
    void updateDefaultPoints();

private:
    // Source points (4 corners)
    double m_srcX1 = 0.0, m_srcY1 = 0.0;      // Top-left
    double m_srcX2 = 100.0, m_srcY2 = 0.0;   // Top-right
    double m_srcX3 = 100.0, m_srcY3 = 100.0; // Bottom-right
    double m_srcX4 = 0.0, m_srcY4 = 100.0;   // Bottom-left

    // Destination points
    double m_dstX1 = 0.0, m_dstY1 = 0.0;      // Top-left
    double m_dstX2 = 100.0, m_dstY2 = 0.0;   // Top-right
    double m_dstX3 = 100.0, m_dstY3 = 100.0; // Bottom-right
    double m_dstX4 = 0.0, m_dstY4 = 100.0;   // Bottom-left

    // Output size
    int m_outputWidth = 640;
    int m_outputHeight = 480;

    // Output options
    int m_interpolation = cv::INTER_LINEAR;  // Interpolation method
    int m_borderMode = cv::BORDER_CONSTANT;  // Border mode
    double m_borderValue = 0.0;              // Border value
    bool m_autoRect = true;                  // Auto-adjust rect to corners

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    int m_inputWidth = 0;
    int m_inputHeight = 0;

    // UI
    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_srcX1Spin = nullptr;
    QDoubleSpinBox* m_srcY1Spin = nullptr;
    QDoubleSpinBox* m_srcX2Spin = nullptr;
    QDoubleSpinBox* m_srcY2Spin = nullptr;
    QDoubleSpinBox* m_srcX3Spin = nullptr;
    QDoubleSpinBox* m_srcY3Spin = nullptr;
    QDoubleSpinBox* m_srcX4Spin = nullptr;
    QDoubleSpinBox* m_srcY4Spin = nullptr;
    QDoubleSpinBox* m_dstX1Spin = nullptr;
    QDoubleSpinBox* m_dstY1Spin = nullptr;
    QDoubleSpinBox* m_dstX2Spin = nullptr;
    QDoubleSpinBox* m_dstY2Spin = nullptr;
    QDoubleSpinBox* m_dstX3Spin = nullptr;
    QDoubleSpinBox* m_dstY3Spin = nullptr;
    QDoubleSpinBox* m_dstX4Spin = nullptr;
    QDoubleSpinBox* m_dstY4Spin = nullptr;
    QSpinBox* m_outputWidthSpin = nullptr;
    QSpinBox* m_outputHeightSpin = nullptr;
    QComboBox* m_interpolationCombo = nullptr;
    QComboBox* m_borderModeCombo = nullptr;
    QDoubleSpinBox* m_borderValueSpin = nullptr;
    QCheckBox* m_autoRectCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_PERSPECTIVETRANSFORMMODEL_H
