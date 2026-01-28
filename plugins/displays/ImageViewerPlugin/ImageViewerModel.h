/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Viewer Node Model
 ******************************************************************************/

#ifndef VISIONBOX_IMAGEVIEWERMODEL_H
#define VISIONBOX_IMAGEVIEWERMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QLabel>
#include <QPixmap>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ImageViewerModel - Displays images in a label widget
 ******************************************************************************/
class ImageViewerModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    ImageViewerModel();
    ~ImageViewerModel() override = default;

    // Node identification
    QString caption() const override { return "Image Viewer"; }
    QString name() const override { return "ImageViewerModel"; }

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

    // Serialization
    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

private slots:
    void updateImage();

private:
    std::shared_ptr<ImageData> m_inputImage;
    QLabel* m_label = nullptr;
    QPixmap m_pixmap;
    bool m_imageChanged = false;
};

} // namespace VisionBox

#endif // VISIONBOX_IMAGEVIEWERMODEL_H
