/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Histogram Equalization Model
 ******************************************************************************/

#ifndef VISIONBOX_HISTOGRAMEQUALIZATIONMODEL_H
#define VISIONBOX_HISTOGRAMEQUALIZATIONMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * HistogramEqualizationModel - Contrast enhancement using histogram equalization
 ******************************************************************************/
class HistogramEqualizationModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    enum EqualizationType {
        Standard,
        CLAHE
    };

    HistogramEqualizationModel();
    ~HistogramEqualizationModel() override = default;

    QString caption() const override { return "Histogram Equalization"; }
    QString name() const override { return "HistogramEqualizationModel"; }

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
    void applyEqualization();
    void onTypeChanged(int index);
    void onClipLimitChanged(int value);
    void onTileSizeChanged(int value);
    void onConvertToGrayChanged(int state);

private:
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_outputImage;
    EqualizationType m_equalizationType = Standard;
    double m_clipLimit = 2.0;
    int m_tileSize = 8;
    bool m_convertToGray = false;

    QWidget* m_widget = nullptr;
    QComboBox* m_typeCombo = nullptr;
    QSpinBox* m_clipLimitSpin = nullptr;
    QSpinBox* m_tileSizeSpin = nullptr;
    QCheckBox* m_convertToGrayCheck = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_HISTOGRAMEQUALIZATIONMODEL_H
