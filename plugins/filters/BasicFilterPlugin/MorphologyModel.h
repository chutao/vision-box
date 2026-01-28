/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Morphology Filter Node Model
 ******************************************************************************/

#ifndef VISIONBOX_MORPHOLOGYMODEL_H
#define VISIONBOX_MORPHOLOGYMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QSpinBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * MorphologyModel - Applies morphological operations
 ******************************************************************************/
class MorphologyModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum MorphOp {
        Erode,
        Dilate,
        Open,
        Close,
        Gradient,
        Tophat,
        Blackhat,
        HitMiss
    };

    MorphologyModel();
    ~MorphologyModel() override = default;

    QString caption() const override { return "Morphology"; }
    QString name() const override { return "MorphologyModel"; }

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
    void applyMorphology();
    void onKernelSizeChanged(int size);
    void onOpChanged(int index);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    int m_kernelSize = 5;
    MorphOp m_operation = Erode;

    QWidget* m_widget = nullptr;
    QComboBox* m_opCombo = nullptr;
    QSpinBox* m_kernelSizeSpin = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_MORPHOLOGYMODEL_H
