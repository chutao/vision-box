/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Contour Plugin Implementation
 ******************************************************************************/

#include "ContourPlugin.h"
#include "ContourFinderModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>>
    ContourPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(1);
    models.push_back(std::unique_ptr<ContourFinderModel>(new ContourFinderModel()));
    return std::move(models);
}

} // namespace VisionBox
