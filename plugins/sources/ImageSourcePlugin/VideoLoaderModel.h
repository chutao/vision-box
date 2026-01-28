/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Video Loader Node Model
 ******************************************************************************/

#ifndef VISIONBOX_VIDEOLOADERMODEL_H
#define VISIONBOX_VIDEOLOADERMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QFileInfo>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * VideoLoaderModel - Loads video files and provides frames
 ******************************************************************************/
class VideoLoaderModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    VideoLoaderModel();
    ~VideoLoaderModel() override;

    // Node identification
    QString caption() const override { return "Video Loader"; }
    QString name() const override { return "VideoLoaderModel"; }

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
    void onPlayPauseClicked();
    void onFrameChanged(int frame);
    void onSliderPressed();
    void onSliderReleased();
    void updateFrame();

private:
    void loadVideo(const QString& filePath);
    void seekToFrame(int frameNumber);
    void updateUI();

private:
    // Video capture
    cv::VideoCapture m_capture;
    QString m_filePath;
    int m_currentFrame = 0;
    int m_totalFrames = 0;
    double m_fps = 30.0;

    // Playback control
    bool m_isPlaying = false;
    bool m_isSeeking = false;
    QTimer* m_playbackTimer = nullptr;

    // Data
    std::shared_ptr<ImageData> m_imageData;

    // UI
    QWidget* m_widget = nullptr;
    QLabel* m_pathLabel = nullptr;
    QLabel* m_frameLabel = nullptr;
    QPushButton* m_browseButton = nullptr;
    QPushButton* m_playPauseButton = nullptr;
    QSlider* m_frameSlider = nullptr;
    QSpinBox* m_frameSpin = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_VIDEOLOADERMODEL_H
