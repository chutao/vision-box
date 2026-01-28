/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Advanced Transform Plugin Implementation
 ******************************************************************************/

#include "AdvancedTransformPlugin.h"
#include "AffineTransformModel.h"
#include "PerspectiveTransformModel.h"

namespace VisionBox {

/*******************************************************************************
 * Node Model Creation
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> AdvancedTransformPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    // Add AffineTransformModel (Phase 15)
    models.push_back(std::unique_ptr<AffineTransformModel>(new AffineTransformModel()));

    // Add PerspectiveTransformModel (Phase 15)
    models.push_back(std::unique_ptr<PerspectiveTransformModel>(new PerspectiveTransformModel()));

    return std::move(models);
}

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/
void AdvancedTransformPlugin::initialize()
{
    // Plugin initialization if needed
}

void AdvancedTransformPlugin::cleanup()
{
    // Plugin cleanup if needed
}

} // namespace VisionBox
