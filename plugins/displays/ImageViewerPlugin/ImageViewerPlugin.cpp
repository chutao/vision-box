/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Viewer Plugin Implementation
 ******************************************************************************/

#include "ImageViewerPlugin.h"
#include "ImageViewerModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ImageViewerPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add ImageViewerModel
    models.push_back(std::unique_ptr<ImageViewerModel>(new ImageViewerModel()));
    return std::move(models);
}

} // namespace VisionBox
