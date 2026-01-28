/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Flip Transform Node Model
 ******************************************************************************/

#ifndef VISIONBOX_FLIPMODEL_H
#define VISIONBOX_FLIPMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * FlipModel - Flip images horizontally, vertically, or both
 ******************************************************************************/
class FlipModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum FlipMode {
        Horizontal,
        Vertical,
        Both
    };

    FlipModel();
    ~FlipModel() override = default;

    QString caption() const override { return "Flip"; }
    QString name() const override { return "FlipModel"; }

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
    void applyFlip();
    void onModeChanged(int index);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    FlipMode m_flipMode = Horizontal;

    QWidget* m_widget = nullptr;
    QComboBox* m_modeCombo = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_FLIPMODEL_H
