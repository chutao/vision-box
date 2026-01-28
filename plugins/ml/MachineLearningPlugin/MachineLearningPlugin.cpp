/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Machine Learning Plugin Implementation
 ******************************************************************************/

#include "MachineLearningPlugin.h"
#include "DNNInferenceModel.h"
#include "ObjectTrackerModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> MachineLearningPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add DNNInferenceModel (Phase 16)
    models.push_back(std::unique_ptr<DNNInferenceModel>(new DNNInferenceModel()));

    // Add ObjectTrackerModel (Phase 16)
    models.push_back(std::unique_ptr<ObjectTrackerModel>(new ObjectTrackerModel()));
    return std::move(models);
}

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/
void MachineLearningPlugin::initialize()
{
    // Plugin initialization if needed
}

void MachineLearningPlugin::cleanup()
{
    // Plugin cleanup if needed
}

} // namespace VisionBox
