/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Affine Transform Model Header
 ******************************************************************************/

#ifndef VISIONBOX_AFFINETRANSFORMMODEL_H
#define VISIONBOX_AFFINETRANSFORMMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * AffineTransformModel - Apply affine transformation to images
 ******************************************************************************/
class AffineTransformModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    AffineTransformModel();
    ~AffineTransformModel() override = default;

    QString caption() const override { return "Affine Transform"; }
    QString name() const override { return "AffineTransformModel"; }

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
    void onRotationChanged(double value);
    void onScaleXChanged(double value);
    void onScaleYChanged(double value);
    void onTranslationXChanged(double value);
    void onTranslationYChanged(double value);
    void onShearXChanged(double value);
    void onShearYChanged(double value);
    void onInterpolationChanged();
    void onBorderModeChanged();
    void onBorderValueChanged(double value);
    void onInvertChanged(int state);

private:
    void applyTransform();

private:
    // Transform parameters
    double m_rotation = 0.0;       // Rotation angle in degrees
    double m_scaleX = 1.0;         // X scale factor
    double m_scaleY = 1.0;         // Y scale factor
    double m_translationX = 0.0;   // X translation in pixels
    double m_translationY = 0.0;   // Y translation in pixels
    double m_shearX = 0.0;         // X shear factor
    double m_shearY = 0.0;         // Y shear factor

    // Output options
    int m_interpolation = cv::INTER_LINEAR;  // Interpolation method
    int m_borderMode = cv::BORDER_CONSTANT;  // Border mode
    double m_borderValue = 0.0;              // Border value
    bool m_invert = false;                   // Invert transformation

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_rotationSpin = nullptr;
    QDoubleSpinBox* m_scaleXSpin = nullptr;
    QDoubleSpinBox* m_scaleYSpin = nullptr;
    QDoubleSpinBox* m_translationXSpin = nullptr;
    QDoubleSpinBox* m_translationYSpin = nullptr;
    QDoubleSpinBox* m_shearXSpin = nullptr;
    QDoubleSpinBox* m_shearYSpin = nullptr;
    QComboBox* m_interpolationCombo = nullptr;
    QComboBox* m_borderModeCombo = nullptr;
    QDoubleSpinBox* m_borderValueSpin = nullptr;
    QCheckBox* m_invertCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_AFFINETRANSFORMMODEL_H
