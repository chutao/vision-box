/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Descriptors Plugin Implementation
 ******************************************************************************/

#include "FeatureDescriptorsPlugin.h"
#include "ORBFeatureModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> FeatureDescriptorsPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<ORBFeatureModel>());
    return std::move(models);
}

} // namespace VisionBox
