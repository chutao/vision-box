/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Optical Flow Plugin Implementation
 ******************************************************************************/

#include "OpticalFlowPlugin.h"
#include "OpticalFlowModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> OpticalFlowPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<OpticalFlowModel>());
    return std::move(models);
}

} // namespace VisionBox
