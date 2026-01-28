/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Distance Transform Plugin Implementation
 ******************************************************************************/

#include "DistanceTransformPlugin.h"
#include "DistanceTransformModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> DistanceTransformPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<DistanceTransformModel>());
    return std::move(models);
}

} // namespace VisionBox
