/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Sobel Edge Detection Node Model
 ******************************************************************************/

#ifndef VISIONBOX_SOBELMODEL_H
#define VISIONBOX_SOBELMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * SobelModel - Sobel edge detection using first-order derivatives
 ******************************************************************************/
class SobelModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    SobelModel();
    ~SobelModel() override = default;

    QString caption() const override { return "Sobel"; }
    QString name() const override { return "SobelModel"; }

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
    void applySobel();
    void onDerivativeChanged(int index);
    void onKernelSizeChanged(int value);
    void onScaleChanged(double value);
    void onDeltaChanged(double value);
    void onConvertToGrayChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    enum DerivativeType {
        XDerivative,
        YDerivative,
        BothDerivatives
    };
    DerivativeType m_derivativeType = BothDerivatives;
    int m_kernelSize = 3;
    double m_scale = 1.0;
    double m_delta = 0.0;
    bool m_convertToGray = true;

    QWidget* m_widget = nullptr;
    QComboBox* m_derivativeCombo = nullptr;
    QSpinBox* m_kernelSizeSpin = nullptr;
    QDoubleSpinBox* m_scaleSpin = nullptr;
    QDoubleSpinBox* m_deltaSpin = nullptr;
    QCheckBox* m_convertToGrayCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_SOBELMODEL_H
