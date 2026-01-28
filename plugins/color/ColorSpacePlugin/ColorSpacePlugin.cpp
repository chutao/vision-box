/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Space Plugin Implementation
 ******************************************************************************/

#include "ColorSpacePlugin.h"
#include "ColorSpaceModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ColorSpacePlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<ColorSpaceModel>());
    return std::move(models);
}

} // namespace VisionBox
