/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Denoise Model
 ******************************************************************************/

#ifndef VISIONBOX_DENOISEMODEL_H
#define VISIONBOX_DENOISEMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * DenoiseModel - Image denoising using various algorithms
 ******************************************************************************/
class DenoiseModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum DenoiseType {
        Bilateral,
        NonLocalMeans,
        FastNlMeans
    };

    DenoiseModel();
    ~DenoiseModel() override = default;

    QString caption() const override { return "Denoise"; }
    QString name() const override { return "DenoiseModel"; }

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
    void applyDenoise();
    void onTypeChanged(int index);
    void onParameter1Changed(double value);
    void onParameter2Changed(double value);
    void onParameter3Changed(int value);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    DenoiseType m_denoiseType = Bilateral;
    double m_param1 = 10.0;  // d or h
    double m_param2 = 30.0;  // sigmaColor or hColor
    int m_param3 = 7;        // templateWindowSize

    QWidget* m_widget = nullptr;
    QComboBox* m_typeCombo = nullptr;
    QDoubleSpinBox* m_param1Spin = nullptr;
    QDoubleSpinBox* m_param2Spin = nullptr;
    QSpinBox* m_param3Spin = nullptr;
    QLabel* m_param1Label = nullptr;
    QLabel* m_param2Label = nullptr;
    QLabel* m_param3Label = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_DENOISEMODEL_H
