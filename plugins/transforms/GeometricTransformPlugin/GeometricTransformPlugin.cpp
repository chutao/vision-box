/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Geometric Transform Plugin Implementation
 ******************************************************************************/

#include "GeometricTransformPlugin.h"
#include "ResizeModel.h"
#include "RotateModel.h"
#include "FlipModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> GeometricTransformPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(3);
    models.push_back(std::make_unique<ResizeModel>());
    models.push_back(std::make_unique<RotateModel>());
    models.push_back(std::make_unique<FlipModel>());

    return std::move(models);
}

} // namespace VisionBox
