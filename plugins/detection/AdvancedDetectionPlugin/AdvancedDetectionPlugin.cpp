/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Detection Plugin Implementation
 ******************************************************************************/

#include "AdvancedDetectionPlugin.h"
#include "YOLOObjectDetectorModel.h"
#include "GrabCutSegmentationModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> AdvancedDetectionPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add YOLOObjectDetectorModel (Phase 18)
    models.push_back(std::unique_ptr<YOLOObjectDetectorModel>(new YOLOObjectDetectorModel()));

    // Add GrabCutSegmentationModel (Phase 18)
    models.push_back(std::unique_ptr<GrabCutSegmentationModel>(new GrabCutSegmentationModel()));

    return std::move(models);
}

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/
void AdvancedDetectionPlugin::initialize()
{
    // Plugin initialization if needed
}

void AdvancedDetectionPlugin::cleanup()
{
    // Plugin cleanup if needed
}

} // namespace VisionBox
