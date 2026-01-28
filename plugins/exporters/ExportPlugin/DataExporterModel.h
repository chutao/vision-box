/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Data Exporter Model Header
 ******************************************************************************/

#ifndef VISIONBOX_DATAEXPORTERMODEL_H
#define VISIONBOX_DATAEXPORTERMODEL_H

#include "core/PluginInterface.h"
#include <QtNodes/NodeDelegateModel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace VisionBox {

class ImageData;

/*******************************************************************************
 * DataExporterModel - Export data to CSV/JSON files
 ******************************************************************************/
class DataExporterModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    DataExporterModel();
    ~DataExporterModel() override = default;

    QString caption() const override { return "Data Exporter"; }
    QString name() const override { return "DataExporterModel"; }

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
    void onBrowseClicked();
    void onExportClicked();
    void onFormatChanged();
    void onExportTypeChanged();
    void onAutoIncrementChanged(int state);
    void onIncludeTimestampChanged(int state);

private:
    bool exportData();
    bool exportToCSV(const QString& filePath);
    bool exportToJSON(const QString& filePath);
    QString generateFileName();
    void collectDataFromImage();
    QString getCurrentTimestamp() const;

private:
    enum ExportFormat
    {
        CSV,
        JSON
    };

    enum ExportType
    {
        ImageInfo,        // Basic image information
        Statistics,       // Image statistics (mean, std dev, etc.)
        Histogram,        // Color histogram data
        DetectionResults  // Detection results (placeholder for future)
    };

    // Export parameters
    QString m_outputPath;          // Output directory
    QString m_filePrefix;           // File name prefix
    int m_formatIndex = 0;           // CSV or JSON
    int m_exportTypeIndex = 0;       // Type of data to export
    bool m_autoIncrement = false;    // Auto-increment filename
    bool m_includeTimestamp = false; // Include timestamp in filename
    int m_frameCount = 0;            // Frame counter for auto-increment

    // Collected data
    struct ImageInfo
    {
        int width = 0;
        int height = 0;
        int channels = 0;
        int depth = 0;
        double minValue = 0.0;
        double maxValue = 0.0;
        double meanValue = 0.0;
        double stdDev = 0.0;
    };
    struct ImageInfo m_imageInfo;
    bool m_hasImageData = false;

    // Data
    std::shared_ptr<ImageData> m_inputImage;

    // UI
    QWidget* m_widget = nullptr;
    QLineEdit* m_pathEdit = nullptr;
    QPushButton* m_browseBtn = nullptr;
    QLineEdit* m_prefixEdit = nullptr;
    QComboBox* m_formatCombo = nullptr;
    QComboBox* m_exportTypeCombo = nullptr;
    QCheckBox* m_autoIncrementCheck = nullptr;
    QCheckBox* m_includeTimestampCheck = nullptr;
    QPushButton* m_exportBtn = nullptr;
    QLabel* m_statusLabel = nullptr;
    QTextEdit* m_previewText = nullptr;
};

} // namespace VisionBox

#endif // VISIONBOX_DATAEXPORTERMODEL_H
