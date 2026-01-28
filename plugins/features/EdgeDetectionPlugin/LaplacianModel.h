/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Laplacian Edge Detection Node Model
 ******************************************************************************/

#ifndef VISIONBOX_LAPLACIANMODEL_H
#define VISIONBOX_LAPLACIANMODEL_H

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
 * LaplacianModel - Laplacian edge detection using second-order derivatives
 ******************************************************************************/
class LaplacianModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum KernelSize {
        Size1,
        Size3,
        Size5,
        Size7
    };

    LaplacianModel();
    ~LaplacianModel() override = default;

    QString caption() const override { return "Laplacian"; }
    QString name() const override { return "LaplacianModel"; }

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
    void applyLaplacian();
    void onKernelSizeChanged(int index);
    void onScaleChanged(double value);
    void onDeltaChanged(double value);
    void onConvertToGrayChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    KernelSize m_kernelSize = Size3;
    double m_scale = 1.0;
    double m_delta = 0.0;
    bool m_convertToGray = true;

    QWidget* m_widget = nullptr;
    QComboBox* m_kernelSizeCombo = nullptr;
    QDoubleSpinBox* m_scaleSpin = nullptr;
    QDoubleSpinBox* m_deltaSpin = nullptr;
    QCheckBox* m_convertToGrayCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_LAPLACIANMODEL_H
