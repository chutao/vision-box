/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Enhancement Plugin Implementation
 ******************************************************************************/

#include "EnhancementPlugin.h"
#include "DenoiseModel.h"
#include "SharpenModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> EnhancementPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(2);
    models.push_back(std::unique_ptr<DenoiseModel>(new DenoiseModel()));
    models.push_back(std::unique_ptr<SharpenModel>(new SharpenModel()));
    return std::move(models);
}

} // namespace VisionBox
