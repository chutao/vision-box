/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Space Model Header
 ******************************************************************************/

#ifndef COLORSPACEMODEL_H
#define COLORSPACEMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

class ImageData;

class ColorSpaceModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum class ColorSpace
    {
        BGR,      // Default OpenCV
        RGB,      // Red, Green, Blue
        HSV,      // Hue, Saturation, Value
        HLS,      // Hue, Lightness, Saturation
        Lab,      // CIE L*a*b*
        Luv,      // CIE L*u*v*
        XYZ,      // CIE XYZ
        YCrCb,    // Y Chroma Red Chroma Blue
        Gray      // Grayscale
    };

    explicit ColorSpaceModel();
    ~ColorSpaceModel() override = default;

    QString caption() const override { return "Color Space"; }
    QString name() const override { return "ColorSpaceModel"; }

public:
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;
    QWidget* embeddedWidget() override;

    QJsonObject save() const;
    void load(QJsonObject const& model) override;

public slots:
    void onColorSpaceChanged(int index);

private:
    void convertColorSpace();

private:
    // Parameters
    ColorSpace m_colorSpace = ColorSpace::RGB;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_colorSpaceCombo = nullptr;
};

} // namespace VisionBox

#endif // COLORSPACEMODEL_H
