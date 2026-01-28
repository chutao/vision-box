/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * K-Means Segmentation Model
 ******************************************************************************/

#ifndef VISIONBOX_KMEANSSEGMENTATIONMODEL_H
#define VISIONBOX_KMEANSSEGMENTATIONMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * KMeansSegmentationModel - Color-based segmentation using K-means clustering
 ******************************************************************************/
class KMeansSegmentationModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum AttemptLevel {
        Low,
        Medium,
        High
    };

    KMeansSegmentationModel();
    ~KMeansSegmentationModel() override = default;

    QString caption() const override { return "K-Means Segmentation"; }
    QString name() const override { return "KMeansSegmentationModel"; }

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
    void applySegmentation();
    void onKChanged(int value);
    void onAttemptsChanged(int index);
    void onRandomCentersChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    int m_k = 3;                    // Number of clusters
    AttemptLevel m_attempts = Medium;
    bool m_useRandomCenters = false;

    QWidget* m_widget = nullptr;
    QSpinBox* m_kSpin = nullptr;
    QComboBox* m_attemptsCombo = nullptr;
    QCheckBox* m_randomCentersCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_KMEANSSEGMENTATIONMODEL_H
