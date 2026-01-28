/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Scharr Edge Detection Node Model
 ******************************************************************************/

#ifndef VISIONBOX_SCHARRMODEL_H
#define VISIONBOX_SCHARRMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ScharrModel - Scharr edge detection (more accurate than Sobel)
 ******************************************************************************/
class ScharrModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum DerivativeType {
        XDerivative,
        YDerivative,
        BothDerivatives
    };

    ScharrModel();
    ~ScharrModel() override = default;

    QString caption() const override { return "Scharr"; }
    QString name() const override { return "ScharrModel"; }

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
    void applyScharr();
    void onDerivativeChanged(int index);
    void onScaleChanged(double value);
    void onDeltaChanged(double value);
    void onConvertToGrayChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    DerivativeType m_derivativeType = BothDerivatives;
    double m_scale = 1.0;
    double m_delta = 0.0;
    bool m_convertToGray = true;

    QWidget* m_widget = nullptr;
    QComboBox* m_derivativeCombo = nullptr;
    QDoubleSpinBox* m_scaleSpin = nullptr;
    QDoubleSpinBox* m_deltaSpin = nullptr;
    QCheckBox* m_convertToGrayCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_SCHARRMODEL_H
