/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Node Error Handling Utilities
 ******************************************************************************/

#ifndef VISIONBOX_NODE_ERROR_H
#define VISIONBOX_NODE_ERROR_H

#include <QString>
#include <QtNodes/internal/NodeDelegateModel.hpp>

// Forward declare cv::Exception to avoid including OpenCV headers
namespace cv {
    class Exception;
}

namespace VisionBox {

/**
 * @brief Error categories for better error classification
 */
enum class ErrorCategory
{
    NoError = 0,           ///< No error
    InvalidInput = 1,      ///< Input data is invalid or missing
    InvalidParameter = 2,  ///< Parameter is out of range or invalid
    ProcessingError = 3,   ///< Error during computation (e.g., OpenCV error)
    MemoryError = 4,       ///< Memory allocation failed
    FileError = 5,         ///< File I/O error
    UnknownError = 6       ///< Unclassified error
};

/**
 * @brief Node error information structure
 */
struct NodeError
{
    ErrorCategory category = ErrorCategory::NoError;
    QString message;
    QString technicalDetails;  ///< Technical details for debugging
    bool recoverable = true;   ///< Whether the error can be recovered

    NodeError() = default;

    NodeError(ErrorCategory cat, const QString& msg, const QString& details = "", bool canRecover = true)
        : category(cat), message(msg), technicalDetails(details), recoverable(canRecover)
    {}

    bool hasError() const { return category != ErrorCategory::NoError; }

    /**
     * @brief Convert to NodeValidationState
     */
    QtNodes::NodeValidationState toValidationState() const
    {
        QtNodes::NodeValidationState state;
        state._state = QtNodes::NodeValidationState::State::Error;

        if (category == ErrorCategory::NoError)
        {
            state._state = QtNodes::NodeValidationState::State::Valid;
            state._stateMessage = "OK";
        }
        else if (category == ErrorCategory::InvalidParameter)
        {
            state._state = QtNodes::NodeValidationState::State::Warning;
            state._stateMessage = message;
        }
        else
        {
            state._state = QtNodes::NodeValidationState::State::Error;
            state._stateMessage = message;
        }

        return state;
    }

    /**
     * @brief Convert to NodeProcessingStatus
     */
    QtNodes::NodeProcessingStatus toProcessingStatus() const
    {
        if (category == ErrorCategory::NoError)
        {
            return QtNodes::NodeProcessingStatus::Updated;
        }
        else if (recoverable)
        {
            return QtNodes::NodeProcessingStatus::Partial;
        }
        else
        {
            return QtNodes::NodeProcessingStatus::Failed;
        }
    }
};

/**
 * @brief Error message builder for user-friendly error messages
 */
class ErrorBuilder
{
public:
    /**
     * @brief Create error for invalid input
     */
    static NodeError invalidInput(const QString& portName, const QString& reason = "")
    {
        QString msg = reason.isEmpty()
            ? QString("Invalid input on port '%1'").arg(portName)
            : QString("Invalid input on port '%1': %2").arg(portName, reason);
        return NodeError(ErrorCategory::InvalidInput, msg, reason, false);
    }

    /**
     * @brief Create error for missing input
     */
    static NodeError missingInput(const QString& portName)
    {
        QString msg = QString("Missing required input: %1").arg(portName);
        return NodeError(ErrorCategory::InvalidInput, msg, "", true);
    }

    /**
     * @brief Create error for invalid parameter
     */
    static NodeError invalidParameter(const QString& paramName, const QString& reason)
    {
        QString msg = QString("Invalid parameter '%1': %2").arg(paramName, reason);
        return NodeError(ErrorCategory::InvalidParameter, msg, reason, true);
    }

    /**
     * @brief Create error for out-of-range parameter
     */
    static NodeError parameterOutOfRange(const QString& paramName,
                                        double value,
                                        double min,
                                        double max)
    {
        QString msg = QString("Parameter '%1' (%2) is out of range [%3, %4]")
                         .arg(paramName)
                         .arg(value)
                         .arg(min)
                         .arg(max);
        return NodeError(ErrorCategory::InvalidParameter, msg, "", true);
    }

    /**
     * @brief Create error for OpenCV exception
     *
     * Note: This function requires opencv2/opencv.hpp to be included before use.
     * Use the generic processingError() if OpenCV headers are not available.
     */
    static NodeError openCVError(const QString& operation, const QString& details)
    {
        QString msg = QString("OpenCV error during %1").arg(operation);
        return NodeError(ErrorCategory::ProcessingError, msg, details, true);
    }

    /**
     * @brief Create error for file operation failure
     */
    static NodeError fileError(const QString& filePath, const QString& operation)
    {
        QString msg = QString("Failed to %1 file: %2").arg(operation, filePath);
        return NodeError(ErrorCategory::FileError, msg, "", false);
    }

    /**
     * @brief Create error for memory allocation failure
     */
    static NodeError memoryError(const QString& what)
    {
        QString msg = QString("Memory allocation failed: %1").arg(what);
        return NodeError(ErrorCategory::MemoryError, msg, "", false);
    }

    /**
     * @brief Create generic processing error
     */
    static NodeError processingError(const QString& operation, const QString& details = "")
    {
        QString msg = QString("Error during %1").arg(operation);
        return NodeError(ErrorCategory::ProcessingError, msg, details, true);
    }

    /**
     * @brief Success indicator (no error)
     */
    static NodeError success()
    {
        return NodeError();
    }
};

/**
 * @brief Base class for nodes with error handling support
 *
 * Derive from this class to add error handling capabilities to your node model.
 */
class ErrorHandlingNode
{
public:
    ErrorHandlingNode() : _lastError() {}

    /**
     * @brief Get the last error
     */
    NodeError lastError() const { return _lastError; }

    /**
     * @brief Check if the last operation resulted in an error
     */
    bool hasError() const { return _lastError.hasError(); }

    /**
     * @brief Clear the error state
     */
    void clearError()
    {
        _lastError = NodeError();
    }

    /**
     * @brief Set error and update validation state
     */
    void setError(const NodeError& error, QtNodes::NodeDelegateModel* model)
    {
        _lastError = error;
        if (model)
        {
            model->setValidationState(error.toValidationState());
            model->setNodeProcessingStatus(error.toProcessingStatus());
        }
    }

    /**
     * @brief Try-catch wrapper for OpenCV operations
     *
     * @return true if operation succeeded, false if error occurred
     */
    template<typename Func>
    bool tryOpenCVOperation(const QString& operation, Func&& func,
                           QtNodes::NodeDelegateModel* model = nullptr)
    {
        try
        {
            func();
            clearError();
            if (model)
            {
                model->setValidationState(ErrorBuilder::success().toValidationState());
                model->setNodeProcessingStatus(ErrorBuilder::success().toProcessingStatus());
            }
            return true;
        }
        catch (const std::exception& e)
        {
            NodeError error = ErrorBuilder::processingError(operation, e.what());
            setError(error, model);
            return false;
        }
        catch (...)
        {
            NodeError error = ErrorBuilder::processingError(operation, "Unknown error");
            setError(error, model);
            return false;
        }
    }

protected:
    NodeError _lastError;
};

} // namespace VisionBox

#endif // VISIONBOX_NODE_ERROR_H
