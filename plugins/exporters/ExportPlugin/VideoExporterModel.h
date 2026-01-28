/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Video Exporter Model Header
 ******************************************************************************/

#ifndef VISIONBOX_VIDEOEXPORTERMODEL_H
#define VISIONBOX_VIDEOEXPORTERMODEL_H

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
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * VideoExporterModel - Save videos to disk
 ******************************************************************************/
class VideoExporterModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    VideoExporterModel();
    ~VideoExporterModel() override;

    QString caption() const override { return "Video Exporter"; }
    QString name() const override { return "VideoExporterModel"; }

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
    void onToggleRecording();
    void onFormatChanged();
    void onFpsChanged(double value);
    void onQualityChanged(int value);

private:
    void initializeWriter();
    void finalizeWriter();
    void writeFrame(const cv::Mat& frame);

private:
    enum State
    {
        Idle,
        Recording,
        Paused
    };

    // Video parameters
    QString m_outputPath;          // Output video file path
    int m_formatIndex = 0;           // Video format
    double m_fps = 30.0;             // Frames per second
    int m_frameSize = 0;              // Frame size from first image
    int m_quality = 95;              // Quality (1-100)

    // Recording state
    State m_state = Idle;
    int m_frameCount = 0;
    cv::VideoWriter m_writer;

    // Data
    std::shared_ptr<ImageData> m_inputImage;

    // UI
    QWidget* m_widget = nullptr;
    QLineEdit* m_pathEdit = nullptr;
    QPushButton* m_browseBtn = nullptr;
    QComboBox* m_formatCombo = nullptr;
    QDoubleSpinBox* m_fpsSpin = nullptr;
    QSpinBox* m_qualitySpin = nullptr;
    QPushButton* m_recordBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_VIDEOEXPORTERMODEL_H
