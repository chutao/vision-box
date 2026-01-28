/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Brightness and Contrast Adjustment Model
 ******************************************************************************/

#ifndef VISIONBOX_BRIGHTNESSCONTRASTMODEL_H
#define VISIONBOX_BRIGHTNESSCONTRASTMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QSlider>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * BrightnessContrastModel - Adjust image brightness and contrast
 ******************************************************************************/
class BrightnessContrastModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    BrightnessContrastModel();
    ~BrightnessContrastModel() override = default;

    QString caption() const override { return "Brightness/Contrast"; }
    QString name() const override { return "BrightnessContrastModel"; }

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
    void applyAdjustment();
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    int m_brightness = 0;      // Range: -100 to 100
    int m_contrast = 0;         // Range: -100 to 100

    QWidget* m_widget = nullptr;
    QSlider* m_brightnessSlider = nullptr;
    QSlider* m_contrastSlider = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_BRIGHTNESSCONTRASTMODEL_H
