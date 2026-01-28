/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Feature Matching Plugin Implementation
 ******************************************************************************/

#include "FeatureMatchingPlugin.h"
#include "SIFTFeaturesModel.h"
#include "AKAZEFeaturesModel.h"
#include "FeatureMatcherModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> FeatureMatchingPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add SIFTFeaturesModel (Phase 14)
    models.push_back(std::unique_ptr<SIFTFeaturesModel>(new SIFTFeaturesModel()));

    // Add AKAZEFeaturesModel (Phase 14) - AKAZE instead of SURF
    models.push_back(std::unique_ptr<AKAZEFeaturesModel>(new AKAZEFeaturesModel()));

    // Add FeatureMatcherModel (Phase 14)
    models.push_back(std::unique_ptr<FeatureMatcherModel>(new FeatureMatcherModel()));
    return std::move(models);
}

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/
void FeatureMatchingPlugin::initialize()
{
    // Plugin initialization if needed
}

void FeatureMatchingPlugin::cleanup()
{
    // Plugin cleanup if needed
}

} // namespace VisionBox
