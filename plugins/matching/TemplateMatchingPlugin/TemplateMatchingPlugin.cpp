/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Template Matching Plugin Implementation
 ******************************************************************************/

#include "TemplateMatchingPlugin.h"
#include "TemplateMatchingModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> TemplateMatchingPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(1);
    models.push_back(std::unique_ptr<TemplateMatchingModel>(new TemplateMatchingModel()));
    return std::move(models);
}

} // namespace VisionBox
