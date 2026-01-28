/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Conversion Node Model
 ******************************************************************************/

#ifndef VISIONBOX_COLORCONVERTMODEL_H
#define VISIONBOX_COLORCONVERTMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ColorConvertModel - Converts between color spaces
 ******************************************************************************/
class ColorConvertModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum ColorSpace {
        BGR,
        RGB,
        HSV,
        HLS,
        YCrCb,
        Grayscale
    };

    ColorConvertModel();
    ~ColorConvertModel() override = default;

    QString caption() const override { return "Color Convert"; }
    QString name() const override { return "ColorConvertModel"; }

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
    void convertColor();
    void onColorSpaceChanged(int index);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    ColorSpace m_targetColorSpace = Grayscale;

    QWidget* m_widget = nullptr;
    QComboBox* m_colorSpaceCombo = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_COLORCONVERTMODEL_H
