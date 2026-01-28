/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Saturation Adjustment Model
 ******************************************************************************/

#ifndef VISIONBOX_SATURATIONMODEL_H
#define VISIONBOX_SATURATIONMODEL_H

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
 * SaturationModel - Adjust color saturation
 ******************************************************************************/
class SaturationModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    SaturationModel();
    ~SaturationModel() override = default;

    QString caption() const override { return "Saturation"; }
    QString name() const override { return "SaturationModel"; }

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
    void applySaturation();
    void onSaturationChanged(int value);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    int m_saturation = 0;      // Range: -100 to 100

    QWidget* m_widget = nullptr;
    QSlider* m_saturationSlider = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_SATURATIONMODEL_H
