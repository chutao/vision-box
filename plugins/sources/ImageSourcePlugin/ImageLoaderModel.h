/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Loader Node Model
 ******************************************************************************/

#ifndef VISIONBOX_IMAGELOADERMODEL_H
#define VISIONBOX_IMAGELOADERMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileInfo>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ImageLoaderModel - Loads images from file
 ******************************************************************************/
class ImageLoaderModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    ImageLoaderModel();
    ~ImageLoaderModel() override = default;

    // Node identification
    QString caption() const override { return "Image Loader"; }
    QString name() const override { return "ImageLoaderModel"; }

    // Port configuration
    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    // Data flow
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override {}

    // Widget
    QWidget* embeddedWidget() override;

    // Serialization
    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

private slots:
    void onBrowseClicked();
    void loadImage(const QString& filePath);

private:
    QString m_filePath;
    std::shared_ptr<ImageData> m_imageData;
    QWidget* m_widget = nullptr;
    QLabel* m_pathLabel = nullptr;
    QPushButton* m_browseButton = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_IMAGELOADERMODEL_H
