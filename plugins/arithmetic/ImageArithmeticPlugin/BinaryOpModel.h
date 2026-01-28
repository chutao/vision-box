/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Binary Operation Node Models
 ******************************************************************************/

#ifndef VISIONBOX_BINARYOPMODEL_H
#define VISIONBOX_BINARYOPMODEL_H

#include <QtNodes/NodeDelegateModel>
#include <QtNodes/NodeData>
#include <QObject>
#include <QString>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QLabel>
#include <opencv2/opencv.hpp>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * BinaryOpModelBase - Base class for binary image operations
 ******************************************************************************/
class BinaryOpModelBase : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    BinaryOpModelBase(const QString& name, const QString& caption,
                      double alpha, double alphaMin, double alphaMax,
                      const QString& alphaLabel);
    ~BinaryOpModelBase() override = default;

    QString caption() const override { return m_caption; }
    QString name() const override { return m_name; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType,
                                   QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data,
                   QtNodes::PortIndex portIndex) override;

    QWidget* embeddedWidget() override;

    QJsonObject save() const override;
    void load(QJsonObject const& model) override;

protected:
    virtual cv::Mat applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha) = 0;

    void applyBinaryOp();

    std::shared_ptr<ImageData> m_inputImage1;
    std::shared_ptr<ImageData> m_inputImage2;
    std::shared_ptr<ImageData> m_outputImage;
    double m_alpha;

    QString m_name;
    QString m_caption;
    QWidget* m_widget = nullptr;
    QDoubleSpinBox* m_alphaSpin = nullptr;

private slots:
    void onAlphaChanged(double value);
};

/*******************************************************************************
 * AddModel
 ******************************************************************************/
class AddModel : public BinaryOpModelBase
{
    Q_OBJECT

public:
    AddModel();

protected:
    cv::Mat applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha) override;
};

/*******************************************************************************
 * SubtractModel
 ******************************************************************************/
class SubtractModel : public BinaryOpModelBase
{
    Q_OBJECT

public:
    SubtractModel();

protected:
    cv::Mat applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha) override;
};

/*******************************************************************************
 * MultiplyModel
 ******************************************************************************/
class MultiplyModel : public BinaryOpModelBase
{
    Q_OBJECT

public:
    MultiplyModel();

protected:
    cv::Mat applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha) override;
};

/*******************************************************************************
 * DivideModel
 ******************************************************************************/
class DivideModel : public BinaryOpModelBase
{
    Q_OBJECT

public:
    DivideModel();

protected:
    cv::Mat applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha) override;
};

/*******************************************************************************
 * AbsDiffModel
 ******************************************************************************/
class AbsDiffModel : public BinaryOpModelBase
{
    Q_OBJECT

public:
    AbsDiffModel();

protected:
    cv::Mat applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha) override;
};

/*******************************************************************************
 * BlendModel
 ******************************************************************************/
class BlendModel : public BinaryOpModelBase
{
    Q_OBJECT

public:
    BlendModel();

protected:
    cv::Mat applyOperation(const cv::Mat& img1, const cv::Mat& img2, double alpha) override;
};

} // namespace VisionBox

#endif // VISIONBOX_BINARYOPMODEL_H
