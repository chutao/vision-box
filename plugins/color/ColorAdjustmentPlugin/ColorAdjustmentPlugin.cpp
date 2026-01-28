/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Color Adjustment Plugin Implementation
 ******************************************************************************/

#include "ColorAdjustmentPlugin.h"
#include "BrightnessContrastModel.h"
#include "SaturationModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> ColorAdjustmentPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(2);
    models.push_back(std::unique_ptr<BrightnessContrastModel>(new BrightnessContrastModel()));
    models.push_back(std::unique_ptr<SaturationModel>(new SaturationModel()));
    return std::move(models);
}

} // namespace VisionBox
