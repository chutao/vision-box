/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Morphology Model Header
 ******************************************************************************/

#ifndef ADVANCEDMORPHOLOGYMODEL_H
#define ADVANCEDMORPHOLOGYMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

class ImageData;

class AdvancedMorphologyModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum class MorphOperation
    {
        TopHat,
        BlackHat,
        Gradient,
        HitMiss
    };

    explicit AdvancedMorphologyModel();
    ~AdvancedMorphologyModel() override = default;

    QString caption() const override { return "Advanced Morphology"; }
    QString name() const override { return "AdvancedMorphologyModel"; }

public:
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;
    QWidget* embeddedWidget() override;

    QJsonObject save() const;
    void load(QJsonObject const& model) override;

public slots:
    void onOperationChanged(int index);
    void onKernelShapeChanged(int index);
    void onKernelSizeChanged(int value);
    void onIterationsChanged(int value);

private:
    void applyMorphology();

private:
    // Parameters
    MorphOperation m_operation = MorphOperation::Gradient;
    int m_kernelShape = 0;  // 0=Rect, 1=Ellipse, 2=Cross
    int m_kernelSize = 5;
    int m_iterations = 1;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_operationCombo = nullptr;
    QComboBox* m_kernelShapeCombo = nullptr;
    QSpinBox* m_kernelSizeSpin = nullptr;
    QSpinBox* m_iterationsSpin = nullptr;
};

} // namespace VisionBox

#endif // ADVANCEDMORPHOLOGYMODEL_H
