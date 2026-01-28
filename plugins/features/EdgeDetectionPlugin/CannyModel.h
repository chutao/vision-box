/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Canny Edge Detection Node Model
 ******************************************************************************/

#ifndef VISIONBOX_CANNYMODEL_H
#define VISIONBOX_CANNYMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * CannyModel - Canny edge detection
 ******************************************************************************/
class CannyModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    CannyModel();
    ~CannyModel() override = default;

    QString caption() const override { return "Canny Edge Detection"; }
    QString name() const override { return "CannyModel"; }

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
    void applyCanny();
    void onThreshold1Changed(double value);
    void onThreshold2Changed(double value);
    void onApertureSizeChanged(int value);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    double m_threshold1 = 50.0;
    double m_threshold2 = 150.0;
    int m_apertureSize = 3;

    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_threshold1Spin = nullptr;
    QDoubleSpinBox* m_threshold2Spin = nullptr;
    QSpinBox* m_apertureSpin = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_CANNYMODEL_H
