/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Contour Finder Model
 ******************************************************************************/

#ifndef VISIONBOX_CONTOURFINDERMODEL_H
#define VISIONBOX_CONTOURFINDERMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ContourFinderModel - Find and draw contours
 ******************************************************************************/
class ContourFinderModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum RetrievalMode {
        External,
        List,
        CComp,
        Tree,
        FloodFill
    };

    enum ContourApproximation {
        None,
        Simple,
        TC89_L1,
        TC89_KCOS
    };

    ContourFinderModel();
    ~ContourFinderModel() override = default;

    QString caption() const override { return "Find Contours"; }
    QString name() const override { return "ContourFinderModel"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;

    QWidget* embeddedWidget() override;

    QJsonObject save() const override;
    void restore(QJsonObject const& model);

private slots:
    void findContours();
    void onModeChanged(int index);
    void onApproxChanged(int index);
    void onMinAreaChanged(int value);
    void onMaxAreaChanged(int value);
    void onDrawContoursChanged(int state);
    void onThicknessChanged(int value);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    RetrievalMode m_retrievalMode = External;
    ContourApproximation m_approximation = None;
    int m_minArea = 0;
    int m_maxArea = 100000;
    bool m_drawContours = true;
    int m_thickness = 1;

    QWidget* m_widget = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QComboBox* m_approxCombo = nullptr;
    QSpinBox* m_minAreaSpin = nullptr;
    QSpinBox* m_maxAreaSpin = nullptr;
    QCheckBox* m_drawContoursCheck = nullptr;
    QSpinBox* m_thicknessSpin = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_CONTOURFINDERMODEL_H
