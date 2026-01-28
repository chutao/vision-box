/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Video Processing Plugin Implementation
 ******************************************************************************/

#include "VideoProcessingPlugin.h"
#include "BackgroundSubtractionModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> VideoProcessingPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<BackgroundSubtractionModel>());
    return std::move(models);
}

} // namespace VisionBox
