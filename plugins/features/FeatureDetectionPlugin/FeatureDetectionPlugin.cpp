/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Detection Plugin Implementation
 ******************************************************************************/

#include "FeatureDetectionPlugin.h"
#include "CornerDetectionModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> FeatureDetectionPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(1);
    models.push_back(std::unique_ptr<CornerDetectionModel>(new CornerDetectionModel()));
    return std::move(models);
}

} // namespace VisionBox
