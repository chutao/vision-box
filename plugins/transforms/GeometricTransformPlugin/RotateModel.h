/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Rotate Transform Node Model
 ******************************************************************************/

#ifndef VISIONBOX_ROTATEMODEL_H
#define VISIONBOX_ROTATEMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * RotateModel - Rotate images by specified angle
 ******************************************************************************/
class RotateModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum InterpolationMode {
        Nearest,
        Linear,
        Cubic,
        Area,
        Lanczos4
    };

    RotateModel();
    ~RotateModel() override = default;

    QString caption() const override { return "Rotate"; }
    QString name() const override { return "RotateModel"; }

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
    void applyRotation();
    void onAngleChanged(double value);
    void onInterpolationChanged(int index);
    void onExpandChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    double m_angle = 0.0;
    InterpolationMode m_interpolation = Linear;
    bool m_expand = false;

    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_angleSpin = nullptr;
    QComboBox* m_interpolationCombo = nullptr;
    QCheckBox* m_expandCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_ROTATEMODEL_H
