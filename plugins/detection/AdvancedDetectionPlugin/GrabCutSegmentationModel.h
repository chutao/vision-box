/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * GrabCut Segmentation Model Header
 ******************************************************************************/

#ifndef VISIONBOX_GRABCUTSEGMENTATIONMODEL_H
#define VISIONBOX_GRABCUTSEGMENTATIONMODEL_H

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
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * GrabCutSegmentationModel - Interactive foreground extraction
 ******************************************************************************/
class GrabCutSegmentationModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    GrabCutSegmentationModel();
    ~GrabCutSegmentationModel() override = default;

    QString caption() const override { return "GrabCut Segmentation"; }
    QString name() const override { return "GrabCutSegmentationModel"; }

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
    void onIterationsChanged(int value);
    void onModeChanged(int index);
    void onUseMaskChanged(int state);
    void onShowMaskChanged(int state);
    void onRunButtonClicked();
    void onResetButtonClicked();

private:
    void runSegmentation();
    void initializeMask();
    cv::Mat applyMask(const cv::Mat& image, const cv::Mat& mask);
    void updateInfoText();

private:
    enum InitMode
    {
        Rect,           // Initialize with rectangle (auto-centered)
        Mask,           // Initialize with mask (requires mask input)
        Auto            // Auto-initialize with threshold
    };

    // GrabCut parameters
    int m_iterations = 5;              // Number of iterations
    int m_initMode = 0;                // Initialization mode
    bool m_useMaskInput = false;       // Use second input as mask
    bool m_showMask = false;           // Show binary mask in output
    bool m_initialized = false;        // Model initialized

    // GrabCut model state
    cv::Mat m_bgdModel;                // Background model
    cv::Mat m_fgdModel;                // Foreground model
    cv::Mat m_mask;                    // Segmentation mask
    cv::Rect m_rect;                   // Initial rectangle

    // Data
    std::shared_ptr<ImageData> m_inputImage;
    std::shared_ptr<ImageData> m_maskImage;
    cv::Mat m_outputImage;

    // UI
    QWidget* m_widget = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QSpinBox* m_iterationsSpin = nullptr;
    QCheckBox* m_useMaskCheck = nullptr;
    QCheckBox* m_showMaskCheck = nullptr;
    QPushButton* m_runBtn = nullptr;
    QPushButton* m_resetBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
    QTextEdit* m_infoText = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_GRABCUTSEGMENTATIONMODEL_H
