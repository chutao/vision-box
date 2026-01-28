/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Watershed Segmentation Plugin Implementation
 ******************************************************************************/

#include "WatershedSegmentationPlugin.h"
#include "WatershedSegmentationModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> WatershedSegmentationPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<WatershedSegmentationModel>());
    return std::move(models);
}

} // namespace VisionBox
