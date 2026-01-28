/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Core Vision Data Types Implementation
 ******************************************************************************/

#include "VisionDataTypes.h"
#include <QImage>
#include <QDebug>

namespace VisionBox {

/*******************************************************************************
 * ImageData Implementation
 ******************************************************************************/
QImage ImageData::toQImage() const
{
    if (m_image.empty())
    {
        qWarning() << "ImageData::toQImage() called on empty cv::Mat";
        return QImage();
    }

    // Convert cv::Mat to QImage based on type
    switch (m_image.type())
    {
        case CV_8UC1:
        {
            // Grayscale 8-bit
            QImage img(m_image.data, m_image.cols, m_image.rows,
                       static_cast<int>(m_image.step), QImage::Format_Grayscale8);
            return img.copy();
        }

        case CV_8UC3:
        {
            // BGR 8-bit (OpenCV default) -> RGB
            QImage img(m_image.data, m_image.cols, m_image.rows,
                       static_cast<int>(m_image.step), QImage::Format_RGB888);
            return img.rgbSwapped();
        }

        case CV_8UC4:
        {
            // BGRA 8-bit -> RGBA
            QImage img(m_image.data, m_image.cols, m_image.rows,
                       static_cast<int>(m_image.step), QImage::Format_ARGB32);
            return img.rgbSwapped();
        }

        case CV_16UC1:
        {
            // 16-bit grayscale - convert to 8-bit
            cv::Mat temp;
            m_image.convertTo(temp, CV_8U, 255.0 / 65535.0);
            QImage img(temp.data, temp.cols, temp.rows,
                      static_cast<int>(temp.step), QImage::Format_Grayscale8);
            return img.copy();
        }

        case CV_32FC1:
        {
            // 32-bit float (0-1) - convert to 8-bit
            cv::Mat temp;
            m_image.convertTo(temp, CV_8U, 255.0);
            QImage img(temp.data, temp.cols, temp.rows,
                      static_cast<int>(temp.step), QImage::Format_Grayscale8);
            return img.copy();
        }

        default:
        {
            qWarning() << "ImageData::toQImage(): Unsupported cv::Mat type:" << m_image.type();

            // Try generic conversion
            cv::Mat temp;
            int cv_type = m_image.type();

            if (m_image.channels() == 1)
            {
                cv_type = CV_8UC1;
            }
            else if (m_image.channels() == 3)
            {
                cv_type = CV_8UC3;
            }
            else if (m_image.channels() == 4)
            {
                cv_type = CV_8UC4;
            }
            else
            {
                return QImage();
            }

            m_image.convertTo(temp, cv_type);
            return ImageData(temp).toQImage();
        }
    }
}

} // namespace VisionBox
