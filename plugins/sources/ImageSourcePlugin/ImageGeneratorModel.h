/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Generator Node Model
 ******************************************************************************/

#ifndef VISIONBOX_IMAGEGENERATORMODEL_H
#define VISIONBOX_IMAGEGENERATORMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * ImageGeneratorModel - Generates test patterns and synthetic images
 ******************************************************************************/
class ImageGeneratorModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    ImageGeneratorModel();
    ~ImageGeneratorModel() override = default;

    // Node identification
    QString caption() const override { return "Image Generator"; }
    QString name() const override { return "ImageGeneratorModel"; }

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
    void onPatternChanged();
    void onParamChanged();
    void performGenerate();  // Debounced image generation

private:
    void generateImage();
    cv::Mat generateSolidColor();
    cv::Mat generateGradient();
    cv::Mat generateCheckerboard();
    cv::Mat generateGrid();
    cv::Mat generateCircles();
    cv::Mat generateRectangles();
    cv::Mat generateNoise();
    cv::Mat generateLines();

private:
    enum PatternType
    {
        SolidColor = 0,
        GradientHorizontal,
        GradientVertical,
        GradientDiagonal,
        Checkerboard,
        Grid,
        Circles,
        Rectangles,
        Lines,
        GaussianNoise,
        UniformNoise
    };

    // Parameters
    PatternType m_pattern = SolidColor;
    int m_width = 640;
    int m_height = 480;
    int m_channels = 3;  // 1 = grayscale, 3 = BGR
    double m_value1 = 128.0;  // For solid color intensity, noise mean, etc.
    double m_value2 = 30.0;   // For noise std, checkerboard size, etc.
    bool m_random = false;    // Random colors

    // Data
    std::shared_ptr<ImageData> m_imageData;

    // Debounce timer to prevent excessive updates
    QTimer* m_generateTimer = nullptr;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_patternCombo = nullptr;
    QSpinBox* m_widthSpin = nullptr;
    QSpinBox* m_heightSpin = nullptr;
    QComboBox* m_channelsCombo = nullptr;
    QDoubleSpinBox* m_value1Spin = nullptr;
    QDoubleSpinBox* m_value2Spin = nullptr;
    QCheckBox* m_randomCheck = nullptr;
    QLabel* m_value1Label = nullptr;
    QLabel* m_value2Label = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_IMAGEGENERATORMODEL_H
