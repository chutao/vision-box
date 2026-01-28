/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Visualization Plugin Implementation
 ******************************************************************************/

#include "VisualizationPlugin.h"
#include "BoundingBoxOverlayModel.h"
#include "KeypointViewerModel.h"
#include "DrawingOverlayModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> VisualizationPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add BoundingBoxOverlayModel (Phase 19)
    models.push_back(std::unique_ptr<BoundingBoxOverlayModel>(new BoundingBoxOverlayModel()));

    // Add KeypointViewerModel (Phase 19)
    models.push_back(std::unique_ptr<KeypointViewerModel>(new KeypointViewerModel()));

    // Add DrawingOverlayModel (Phase 19)
    models.push_back(std::unique_ptr<DrawingOverlayModel>(new DrawingOverlayModel()));
    return std::move(models);
}

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/
void VisualizationPlugin::initialize()
{
    // Plugin initialization if needed
}

void VisualizationPlugin::cleanup()
{
    // Plugin cleanup if needed
}

} // namespace VisionBox
