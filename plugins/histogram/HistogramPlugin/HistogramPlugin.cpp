/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Histogram Plugin Implementation
 ******************************************************************************/

#include "HistogramPlugin.h"
#include "HistogramEqualizationModel.h"

namespace VisionBox {

/*******************************************************************************
 * Create Node Models
 ******************************************************************************/
std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> HistogramPlugin::createNodeModels() const
{
    std::vector<std::unique_ptr<::QtNodes::NodeDelegateModel>> models;

    models.reserve(1);
    models.push_back(std::unique_ptr<HistogramEqualizationModel>(new HistogramEqualizationModel()));
    return std::move(models);
}

} // namespace VisionBox
