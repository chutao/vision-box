/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Edge Detection Plugin Implementation
 ******************************************************************************/

#include "EdgeDetectionPlugin.h"
#include "CannyModel.h"
#include "SobelModel.h"
#include "LaplacianModel.h"
#include "ScharrModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> EdgeDetectionPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(4);
    models.push_back(std::unique_ptr<CannyModel>(new CannyModel()));
    models.push_back(std::unique_ptr<SobelModel>(new SobelModel()));
    models.push_back(std::unique_ptr<LaplacianModel>(new LaplacianModel()));
    models.push_back(std::unique_ptr<ScharrModel>(new ScharrModel()));
    return std::move(models);
}

} // namespace VisionBox
