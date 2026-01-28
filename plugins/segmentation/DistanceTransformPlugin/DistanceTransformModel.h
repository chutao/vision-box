/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Distance Transform Model Header
 ******************************************************************************/

#ifndef DISTANCETRANSFORMMODEL_H
#define DISTANCETRANSFORMMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

class ImageData;

class DistanceTransformModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum class DistanceType
    {
        L1,      // City block / Manhattan distance
        L2,      // Euclidean distance
        C        // Checkerboard distance
    };

    enum class LabelType
    {
        Distance,
        Labels
    };

    explicit DistanceTransformModel();
    ~DistanceTransformModel() override = default;

    QString caption() const override { return "Distance Transform"; }
    QString name() const override { return "DistanceTransformModel"; }

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
    void onDistanceTypeChanged(int index);
    void onLabelTypeChanged(int index);
    void onNormalizeChanged(int state);

private:
    void computeDistanceTransform();

private:
    // Parameters
    DistanceType m_distanceType = DistanceType::L2;
    LabelType m_labelType = LabelType::Distance;
    bool m_normalize = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_distanceTypeCombo = nullptr;
    QComboBox* m_labelTypeCombo = nullptr;
    QCheckBox* m_normalizeCheck = nullptr;
};

} // namespace VisionBox

#endif // DISTANCETRANSFORMMODEL_H
