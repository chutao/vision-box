/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Image Pyramid Plugin Implementation
 ******************************************************************************/

#include "ImagePyramidPlugin.h"
#include "ImagePyramidModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ImagePyramidPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<ImagePyramidModel>());

    return std::move(models);
}

} // namespace VisionBox
