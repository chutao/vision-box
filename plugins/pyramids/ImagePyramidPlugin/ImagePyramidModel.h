/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Pyramid Model Header
 ******************************************************************************/

#ifndef IMAGEPYRAMIDMODEL_H
#define IMAGEPYRAMIDMODEL_H

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

class ImagePyramidModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum class PyramidType
    {
        Gaussian,
        Laplacian
    };

    explicit ImagePyramidModel();
    ~ImagePyramidModel() override = default;

    QString caption() const override { return "Image Pyramid"; }
    QString name() const override { return "ImagePyramidModel"; }

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
    void onPyramidTypeChanged(int index);
    void onLevelsChanged(int value);
    void onDisplayLevelChanged(int value);

private:
    void buildPyramid();

private:
    // Parameters
    PyramidType m_pyramidType = PyramidType::Gaussian;
    int m_levels = 4;
    int m_displayLevel = 0;  // Which pyramid level to display

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;

    // Pyramid storage
    std::vector<cv::Mat> m_gaussianPyramid;
    std::vector<cv::Mat> m_laplacianPyramid;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_pyramidTypeCombo = nullptr;
    QSpinBox* m_levelsSpin = nullptr;
    QSpinBox* m_displayLevelSpin = nullptr;
};

} // namespace VisionBox

#endif // IMAGEPYRAMIDMODEL_H
