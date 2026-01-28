/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Resize Transform Node Model
 ******************************************************************************/

#ifndef VISIONBOX_RESIZEMODEL_H
#define VISIONBOX_RESIZEMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ResizeModel - Resize images to specific dimensions
 ******************************************************************************/
class ResizeModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum ResizeMode {
        Absolute,
        Relative,
        FitWidth,
        FitHeight
    };

    ResizeModel();
    ~ResizeModel() override = default;

    QString caption() const override { return "Resize"; }
    QString name() const override { return "ResizeModel"; }

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
    void applyResize();
    void onWidthChanged(int value);
    void onHeightChanged(int value);
    void onModeChanged(int index);
    void onKeepAspectChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    ResizeMode m_resizeMode = Absolute;
    int m_targetWidth = 640;
    int m_targetHeight = 480;
    bool m_keepAspectRatio = false;

    QWidget* m_widget = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QSpinBox* m_widthSpin = nullptr;
    QSpinBox* m_heightSpin = nullptr;
    QCheckBox* m_keepAspectCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_RESIZEMODEL_H
