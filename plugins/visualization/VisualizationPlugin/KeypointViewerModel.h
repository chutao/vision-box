/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Keypoint Viewer Model Header
 ******************************************************************************/

#ifndef VISIONBOX_KEYPOINTVIEWERMODEL_H
#define VISIONBOX_KEYPOINTVIEWERMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QJsonArray>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * KeypointViewerModel - Visualize feature points and keypoints
 ******************************************************************************/
class KeypointViewerModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    KeypointViewerModel();
    ~KeypointViewerModel() override = default;

    QString caption() const override { return "Keypoint Viewer"; }
    QString name() const override { return "KeypointViewerModel"; }

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
    void onPointRadiusChanged(int value);
    void onPointStyleChanged(int index);
    void onColorModeChanged(int index);
    void onShowConnectionsChanged(int state);
    void onDrawCirclesChanged(int state);
    void onFixedColorChanged();

private:
    void drawKeypoints();
    cv::Scalar getColorForIndex(int index);
    void updateInfoText();

private:
    // Drawing parameters
    int m_pointRadius = 3;               // Point/circle radius
    int m_pointStyle = 0;                // 0=Circle, 1=Cross, 2=Plus
    int m_colorMode = 0;                 // 0=By Index, 1=Single Color, 2=Random
    bool m_showConnections = false;      // Draw lines between consecutive points
    bool m_drawCircles = true;           // Draw filled circles

    // Color settings
    cv::Scalar m_fixedColor = cv::Scalar(0, 0, 255);  // Red (BGR)

    // Keypoints data
    std::vector<cv::KeyPoint> m_keypoints;
    std::vector<cv::Point2f> m_points;
    bool m_hasKeypoints = false;

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    cv::Mat m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QSpinBox* m_radiusSpin = nullptr;
    QComboBox* m_styleCombo = nullptr;
    QComboBox* m_colorModeCombo = nullptr;
    QPushButton* m_colorBtn = nullptr;
    QCheckBox* m_showConnectionsCheck = nullptr;
    QCheckBox* m_drawCirclesCheck = nullptr;
    QTextEdit* m_infoText = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_KEYPOINTVIEWERMODEL_H
