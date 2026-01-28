/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Segmentation Plugin Implementation
 ******************************************************************************/

#include "SegmentationPlugin.h"
#include "KMeansSegmentationModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> SegmentationPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(1);
    models.push_back(std::unique_ptr<KMeansSegmentationModel>(new KMeansSegmentationModel()));
    return std::move(models);
}

} // namespace VisionBox
