/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Face Detection Plugin Implementation
 ******************************************************************************/

#include "FaceDetectionPlugin.h"
#include "HaarFaceDetectionModel.h"
#include <QtNodes/NodeDelegateModel>

namespace VisionBox {

std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> FaceDetectionPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;
    models.reserve(1);

    models.push_back(std::make_unique<HaarFaceDetectionModel>());
    return std::move(models);
}

} // namespace VisionBox
