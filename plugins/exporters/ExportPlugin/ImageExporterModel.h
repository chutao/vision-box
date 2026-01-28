/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Exporter Model Header
 ******************************************************************************/

#ifndef VISIONBOX_IMAGEEXPORTERMODEL_H
#define VISIONBOX_IMAGEEXPORTERMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ImageExporterModel - Save images to disk
 ******************************************************************************/
class ImageExporterModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    ImageExporterModel();
    ~ImageExporterModel() override = default;

    QString caption() const override { return "Image Exporter"; }
    QString name() const override { return "ImageExporterModel"; }

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
    void onBrowseClicked();
    void onExportClicked();
    void onFormatChanged();
    void onQualityChanged(int value);
    void onAutoIncrementChanged(int state);

private:
    bool exportImage(const cv::Mat& image);
    QString generateFileName();

private:
    // Export parameters
    QString m_outputPath;          // Output directory
    QString m_filePrefix;           // File name prefix
    int m_formatIndex = 0;           // Output format
    int m_quality = 95;              // JPEG quality (1-100)
    bool m_autoIncrement = false;     // Auto-increment filename
    int m_frameCount = 0;            // Frame counter for auto-increment

    // Format mappings
    QMap<int, QString> m_formatExtensions;

    // Data
    std::shared_ptr<ImageData> m_inputImage;

    // UI
    QWidget* m_widget = nullptr;
    QLineEdit* m_pathEdit = nullptr;
    QPushButton* m_browseBtn = nullptr;
    QLineEdit* m_prefixEdit = nullptr;
    QComboBox* m_formatCombo = nullptr;
    QSpinBox* m_qualitySpin = nullptr;
    QCheckBox* m_autoIncrementCheck = nullptr;
    QPushButton* m_exportBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_IMAGEEXPORTERMODEL_H
