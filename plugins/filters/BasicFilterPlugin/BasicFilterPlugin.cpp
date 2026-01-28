/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Basic Filter Plugin Implementation
 ******************************************************************************/

#include "BasicFilterPlugin.h"
#include "BlurModel.h"
#include "ThresholdModel.h"
#include "MorphologyModel.h"
#include "ColorConvertModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> BasicFilterPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add BlurModel
    models.push_back(std::unique_ptr<BlurModel>(new BlurModel()));

    // Add ThresholdModel
    models.push_back(std::unique_ptr<ThresholdModel>(new ThresholdModel()));

    // Add MorphologyModel
    models.push_back(std::unique_ptr<MorphologyModel>(new MorphologyModel()));

    // Add ColorConvertModel
    models.push_back(std::unique_ptr<ColorConvertModel>(new ColorConvertModel()));

    return std::move(models);
}

} // namespace VisionBox
