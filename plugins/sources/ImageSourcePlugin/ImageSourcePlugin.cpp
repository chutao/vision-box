/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Source Plugin Implementation
 ******************************************************************************/

#include "ImageSourcePlugin.h"
#include "ImageLoaderModel.h"
#include "VideoLoaderModel.h"
#include "CameraSourceModel.h"
#include "ImageGeneratorModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ImageSourcePlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add ImageLoaderModel
    models.push_back(std::unique_ptr<ImageLoaderModel>(new ImageLoaderModel()));

    // Add VideoLoaderModel (Phase 13)
    models.push_back(std::unique_ptr<VideoLoaderModel>(new VideoLoaderModel()));

    // Add CameraSourceModel (Phase 13)
    models.push_back(std::unique_ptr<CameraSourceModel>(new CameraSourceModel()));

    // Add ImageGeneratorModel (Phase 13)
    models.push_back(std::unique_ptr<ImageGeneratorModel>(new ImageGeneratorModel()));
    return std::move(models);
}

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/
void ImageSourcePlugin::initialize()
{
    // Plugin initialization if needed
}

void ImageSourcePlugin::cleanup()
{
    // Plugin cleanup if needed
}

} // namespace VisionBox
