/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Export Plugin Implementation
 ******************************************************************************/

#include "ExportPlugin.h"
#include "ImageExporterModel.h"
#include "VideoExporterModel.h"
#include "DataExporterModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ExportPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add ImageExporterModel (Phase 17)
    models.push_back(std::unique_ptr<ImageExporterModel>(new ImageExporterModel()));

    // Add VideoExporterModel (Phase 17)
    models.push_back(std::unique_ptr<VideoExporterModel>(new VideoExporterModel()));

    // Add DataExporterModel (Phase 17)
    models.push_back(std::unique_ptr<DataExporterModel>(new DataExporterModel()));
    return std::move(models);
}

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/
void ExportPlugin::initialize()
{
    // Plugin initialization if needed
}

void ExportPlugin::cleanup()
{
    // Plugin cleanup if needed
}

} // namespace VisionBox
