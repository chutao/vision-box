/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Threshold Filter Node Model
 ******************************************************************************/

#ifndef VISIONBOX_THRESHOLDMODEL_H
#define VISIONBOX_THRESHOLDMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QSpinBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ThresholdModel - Applies thresholding to images
 ******************************************************************************/
class ThresholdModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum ThresholdType {
        Binary,
        BinaryInv,
        Trunc,
        ToZero,
        ToZeroInv,
        Otsu,
        AdaptiveMean,
        AdaptiveGaussian
    };

    ThresholdModel();
    ~ThresholdModel() override = default;

    QString caption() const override { return "Threshold"; }
    QString name() const override { return "ThresholdModel"; }

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
    void applyThreshold();
    void onThresholdValueChanged(int value);
    void onMaxValueChanged(int value);
    void onTypeChanged(int index);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    double m_thresholdValue = 127.0;
    double m_maxValue = 255.0;
    ThresholdType m_thresholdType = Binary;

    QWidget* m_widget = nullptr;
    QComboBox* m_typeCombo = nullptr;
    QSpinBox* m_thresholdSpin = nullptr;
    QSpinBox* m_maxSpin = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_THRESHOLDMODEL_H
