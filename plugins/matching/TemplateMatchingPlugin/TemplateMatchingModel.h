/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Template Matching Model
 ******************************************************************************/

#ifndef VISIONBOX_TEMPLATEMATCHINGMODEL_H
#define VISIONMATCHINGMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * TemplateMatchingModel - Find template in image using template matching
 ******************************************************************************/
class TemplateMatchingModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum MatchingMethod {
        SquaredDifference,
        NormalizedCrossCorrelation,
        CorrelationCoefficient,
        NormalizedSquaredDifference
    };

    TemplateMatchingModel();
    ~TemplateMatchingModel() override = default;

    QString caption() const override { return "Template Matching"; }
    QString name() const override { return "TemplateMatchingModel"; }

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
    void applyMatching();
    void onMethodChanged(int index);
    void onThresholdChanged(double value);
    void onDrawBoxChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_templateImage;
    std::shared_ptr<ImageData> m_outputImage;
    MatchingMethod m_method = NormalizedCrossCorrelation;
    double m_threshold = 0.8;
    bool m_drawBox = true;

    QWidget* m_widget = nullptr;
    QComboBox* m_methodCombo = nullptr;
    QDoubleSpinBox* m_thresholdSpin = nullptr;
    QCheckBox* m_drawBoxCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_TEMPLATEMATCHINGMODEL_H
