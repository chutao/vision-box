/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Watershed Segmentation Model Header
 ******************************************************************************/

#ifndef WATERSHEDSEGMENTATIONMODEL_H
#define WATERSHEDSEGMENTATIONMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace VisionBox {

class ImageData;

class WatershedSegmentationModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    explicit WatershedSegmentationModel();
    ~WatershedSegmentationModel() override = default;

    QString caption() const override { return "Watershed Segmentation"; }
    QString name() const override { return "WatershedSegmentationModel"; }

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
    void onMarkersChanged(int value);
    void onIterationsChanged(int value);
    void onColorRegionsChanged(int state);

private:
    void performWatershed();

private:
    // Parameters
    int m_markers = 8;  // Number of automatic markers to place
    int m_iterations = 5;
    bool m_colorRegions = true;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QSpinBox* m_markersSpin = nullptr;
    QSpinBox* m_iterationsSpin = nullptr;
    QCheckBox* m_colorRegionsCheck = nullptr;
};

} // namespace VisionBox

#endif // WATERSHEDSEGMENTATIONMODEL_H
