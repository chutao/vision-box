/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Morphology Plugin Implementation
 ******************************************************************************/

#include "AdvancedMorphologyPlugin.h"
#include "AdvancedMorphologyModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> AdvancedMorphologyPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<AdvancedMorphologyModel>());
    return std::move(models);
}

} // namespace VisionBox
