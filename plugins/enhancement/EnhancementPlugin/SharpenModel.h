/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Sharpen Model
 ******************************************************************************/

#ifndef VISIONBOX_SHARPENMODEL_H
#define VISIONBOX_SHARPENMODEL_H

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
 * SharpenModel - Image sharpening using unsharp mask
 ******************************************************************************/
class SharpenModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    SharpenModel();
    ~SharpenModel() override = default;

    QString caption() const override { return "Sharpen"; }
    QString name() const override { return "SharpenModel"; }

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
    void applySharpening();
    void onStrengthChanged(int value);
    void onRadiusChanged(int value);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    double m_strength = 1.0;  // Amount of sharpening (0.0 to 5.0)
    int m_radius = 1;         // Gaussian radius (1 to 10)

    QWidget* m_widget = nullptr;
    QSlider* m_strengthSlider = nullptr;
    QSlider* m_radiusSlider = nullptr;
    QLabel* m_strengthValueLabel = nullptr;
    QLabel* m_radiusValueLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_SHARPENMODEL_H
