/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Arithmetic Plugin Implementation
 ******************************************************************************/

#include "ImageArithmeticPlugin.h"
#include "BinaryOpModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ImageArithmeticPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(6);
    models.push_back(std::make_unique<AddModel>());
    models.push_back(std::make_unique<SubtractModel>());
    models.push_back(std::make_unique<MultiplyModel>());
    models.push_back(std::make_unique<DivideModel>());
    models.push_back(std::make_unique<AbsDiffModel>());
    models.push_back(std::make_unique<BlendModel>());
    return std::move(models);
}

} // namespace VisionBox
