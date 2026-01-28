/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Blur Filter Node Model
 ******************************************************************************/

#ifndef VISIONBOX_BLURMODEL_H
#define VISIONBOX_BLURMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QSpinBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include "core/NodeError.h"

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * BlurModel - Applies Gaussian or median blur to images
 ******************************************************************************/
class BlurModel : public QtNodes::NodeDelegateModel, public ErrorHandlingNode
{
    Q_OBJECT

public:
    enum BlurType { Gaussian, Median };

    BlurModel();
    ~BlurModel() override = default;

    // Node identification
    QString caption() const override { return "Blur Filter"; }
    QString name() const override { return "BlurModel"; }
    QString portCaption(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    // Port configuration
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    // Data flow
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;

    // Widget
    QWidget* embeddedWidget() override;

    // Validation
    QtNodes::NodeValidationState validationState() const override;

    // Serialization
    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

private slots:
    void applyBlur();
    void onKernelSizeChanged(int size);
    void onBlurTypeChanged(int index);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    int m_kernelSize = 5;
    BlurType m_blurType = Gaussian;

    QWidget* m_widget = nullptr;
    QComboBox* m_blurTypeCombo = nullptr;
    QSpinBox* m_kernelSizeSpin = nullptr;

    // Validation state
    QtNodes::NodeValidationState m_validationState;
};

} // namespace VisionBox

#endif // VISIONBOX_BLURMODEL_H
