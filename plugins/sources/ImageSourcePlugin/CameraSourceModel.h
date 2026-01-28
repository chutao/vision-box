/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Camera Source Node Model
 ******************************************************************************/

#ifndef VISIONBOX_CAMERASOURCEMODEL_H
#define VISIONBOX_CAMERASOURCEMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * CameraSourceModel - Captures frames from camera
 ******************************************************************************/
class CameraSourceModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    CameraSourceModel();
    ~CameraSourceModel() override;

    // Node identification
    QString caption() const override { return "Camera Source"; }
    QString name() const override { return "CameraSourceModel"; }

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
    void onOpenCamera();
    void onCloseCamera();
    void onCameraIdChanged(int id);
    void onResolutionChanged();
    void captureFrame();

private:
    void updateCameraList();
    void updateUI();
    QString getResolutionString(int width, int height);

private:
    // Camera capture
    cv::VideoCapture m_capture;
    int m_cameraId = 0;
    int m_width = 640;
    int m_height = 480;
    bool m_isOpened = false;

    // Frame capture timer
    QTimer* m_captureTimer = nullptr;

    // Data
    std::shared_ptr<ImageData> m_imageData;

    // UI
    QWidget* m_widget = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_resolutionLabel = nullptr;
    QComboBox* m_cameraCombo = nullptr;
    QSpinBox* m_cameraIdSpin = nullptr;
    QComboBox* m_resolutionCombo = nullptr;
    QPushButton* m_openButton = nullptr;
    QPushButton* m_closeButton = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_CAMERASOURCEMODEL_H
