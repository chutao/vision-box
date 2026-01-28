/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Corner Detection Model
 ******************************************************************************/

#ifndef VISIONBOX_CORNERDETECTIONMODEL_H
#define VISIONBOX_CORNERDETECTIONMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * CornerDetectionModel - Corner detection using Harris or Shi-Tomasi
 ******************************************************************************/
class CornerDetectionModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum CornerMethod {
        Harris,
        ShiTomasi
    };

    CornerDetectionModel();
    ~CornerDetectionModel() override = default;

    QString caption() const override { return "Corner Detection"; }
    QString name() const override { return "CornerDetectionModel"; }

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
    void detectCorners();
    void onMethodChanged(int index);
    void onQualityLevelChanged(double value);
    void onMaxCornersChanged(int value);
    void onMinDistanceChanged(int value);
    void onBlockSizeChanged(int value);
    void onShowCornersChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    CornerMethod m_method = Harris;
    double m_qualityLevel = 0.01;  // Quality threshold
    int m_maxCorners = 100;          // Maximum corners to detect
    int m_minDistance = 10;           // Minimum distance between corners
    int m_blockSize = 3;              // Neighborhood size
    bool m_showCorners = true;

    QWidget* m_widget = nullptr;
    QComboBox* m_methodCombo = nullptr;
    QDoubleSpinBox* m_qualitySpin = nullptr;
    QSpinBox* m_maxCornersSpin = nullptr;
    QSpinBox* m_minDistanceSpin = nullptr;
    QSpinBox* m_blockSizeSpin = nullptr;
    QCheckBox* m_showCornersCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_CORNERDETECTIONMODEL_H
