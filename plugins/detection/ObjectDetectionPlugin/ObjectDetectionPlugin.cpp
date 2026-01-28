/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Object Detection Plugin Implementation
 ******************************************************************************/

#include "ObjectDetectionPlugin.h"
#include "HOGDetectionModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ObjectDetectionPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<HOGDetectionModel>());

    return std::move(models);
}

} // namespace VisionBox
