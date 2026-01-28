/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Unit Tests for Vision Data Types
 ******************************************************************************/

#include <QtTest/QtTest>
#include <opencv2/core/mat.hpp>
#include "core/VisionDataTypes.h"

using namespace VisionBox;

/*******************************************************************************
 * Test Suite: ImageData Tests
 ******************************************************************************/
class ImageDataTest : public QObject
{
    Q_OBJECT

private slots:
    /***************************************************************************
     * Construction and Initialization
     **************************************************************************/
    void testDefaultConstruction()
    {
        ImageData data;
        QVERIFY(data.image().empty());
        QVERIFY(!data.isValid());
        QCOMPARE(data.width(), 0);
        QCOMPARE(data.height(), 0);
        QCOMPARE(data.channels(), 0);
    }

    void testConstructionFromMat()
    {
        cv::Mat mat(100, 200, CV_8UC3);
        ImageData data(mat);

        QVERIFY(!data.image().empty());
        QVERIFY(data.isValid());
        QCOMPARE(data.width(), 200);
        QCOMPARE(data.height(), 100);
        QCOMPARE(data.channels(), 3);
    }

    /***************************************************************************
     * Type ID Verification
     **************************************************************************/
    void testTypeId()
    {
        ImageData data;
        ::QtNodes::NodeDataType dataType = data.type();

        QCOMPARE(dataType.id, QString("opencv_image"));
        QCOMPARE(dataType.name, QString("Image"));
    }

    /***************************************************************************
     * Data Access and Conversion
     **************************************************************************/
    void testImageAccess()
    {
        cv::Mat mat(50, 100, CV_8UC1, cv::Scalar(128));
        ImageData data(mat);

        cv::Mat retrieved = data.image();
        QCOMPARE(retrieved.rows, 50);
        QCOMPARE(retrieved.cols, 100);
        QCOMPARE(retrieved.type(), CV_8UC1);
    }

    void testSetImage()
    {
        ImageData data;
        QVERIFY(data.image().empty());

        cv::Mat mat1(100, 100, CV_8UC3);
        data.setImage(mat1);
        QVERIFY(data.isValid());
        QCOMPARE(data.channels(), 3);

        cv::Mat mat2(50, 50, CV_8UC1);
        data.setImage(mat2);
        QCOMPARE(data.channels(), 1);
        QCOMPARE(data.width(), 50);
    }

    /***************************************************************************
     * Memory Management (Reference Counting)
     **************************************************************************/
    void testMatReferenceCounting()
    {
        cv::Mat mat(100, 100, CV_8UC3);
        mat.setTo(cv::Scalar(255, 0, 0));

        ImageData data1(mat);
        ImageData data2 = data1; // Copy constructor

        // Both should point to the same underlying data
        cv::Mat img1 = data1.image();
        cv::Mat img2 = data2.image();

        // Modify through img1
        img1.setTo(cv::Scalar(0, 255, 0));

        // Check that img2 also sees the change (same data)
        cv::Vec3b pixel = img2.at<cv::Vec3b>(50, 50);
        QCOMPARE(pixel[0], 0);   // B
        QCOMPARE(pixel[1], 255); // G
        QCOMPARE(pixel[2], 0);   // R
    }

    void testDeepCopy()
    {
        cv::Mat mat(100, 100, CV_8UC3);
        mat.setTo(cv::Scalar(255, 0, 0));

        ImageData data1(mat);

        // Get a copy of the image
        cv::Mat mat1 = data1.image();
        cv::Mat mat2 = mat1.clone(); // Explicit deep copy

        // Modify mat1
        mat1.setTo(cv::Scalar(0, 255, 0));

        // mat2 should be unchanged
        cv::Vec3b pixel = mat2.at<cv::Vec3b>(50, 50);
        QCOMPARE(pixel[0], 255); // B
        QCOMPARE(pixel[1], 0);   // G
        QCOMPARE(pixel[2], 0);   // R
    }
};

/*******************************************************************************
 * Test Suite: DetectionData Tests
 ******************************************************************************/
class DetectionDataTest : public QObject
{
    Q_OBJECT

private slots:
    /***************************************************************************
     * Construction and Initialization
     **************************************************************************/
    void testDefaultConstruction()
    {
        DetectionData data;
        QVERIFY(data.isEmpty());
        QCOMPARE(data.count(), 0);
    }

    void testConstructionFromVector()
    {
        QVector<DetectionData::Detection> detections;
        detections.append(
            DetectionData::Detection(QRectF(0.1, 0.2, 0.3, 0.4), "person", 0.95));
        detections.append(
            DetectionData::Detection(QRectF(0.5, 0.6, 0.2, 0.3), "car", 0.87));

        DetectionData data(detections);
        QVERIFY(!data.isEmpty());
        QCOMPARE(data.count(), 2);
    }

    /***************************************************************************
     * Type ID Verification
     **************************************************************************/
    void testTypeId()
    {
        DetectionData data;
        ::QtNodes::NodeDataType dataType = data.type();

        QCOMPARE(dataType.id, QString("detection"));
        QCOMPARE(dataType.name, QString("Detection"));
    }

    /***************************************************************************
     * Adding and Retrieving Detections
     **************************************************************************/
    void testAddDetection()
    {
        DetectionData data;

        data.addDetection(QRectF(0.1, 0.2, 0.3, 0.4), "person", 0.95);

        QCOMPARE(data.count(), 1);
        QVERIFY(!data.isEmpty());

        DetectionData::Detection det = data.detections().first();
        QCOMPARE(det.bbox, QRectF(0.1, 0.2, 0.3, 0.4));
        QCOMPARE(det.label, QString("person"));
        QCOMPARE(det.confidence, 0.95f);
    }

    void testAddMultipleDetections()
    {
        DetectionData data;

        for (int i = 0; i < 5; ++i)
        {
            data.addDetection(QRectF(i * 0.1, i * 0.1, 0.1, 0.1),
                QString("object%1").arg(i),
                0.9 - i * 0.1);
        }

        QCOMPARE(data.count(), 5);
    }

    void testRetrieveDetections()
    {
        QVector<DetectionData::Detection> detections;
        detections.append(
            DetectionData::Detection(QRectF(0.1, 0.2, 0.3, 0.4), "person", 0.95));
        detections.append(
            DetectionData::Detection(QRectF(0.5, 0.6, 0.2, 0.3), "car", 0.87));

        DetectionData data(detections);

        QVector<DetectionData::Detection> retrieved = data.detections();
        QCOMPARE(retrieved.size(), 2);

        QCOMPARE(retrieved[0].label, QString("person"));
        QCOMPARE(retrieved[1].label, QString("car"));
    }

    /***************************************************************************
     * Clear and Empty State
     **************************************************************************/
    void testClear()
    {
        DetectionData data;
        data.addDetection(QRectF(0.1, 0.2, 0.3, 0.4), "person", 0.95);
        data.addDetection(QRectF(0.5, 0.6, 0.2, 0.3), "car", 0.87);

        QCOMPARE(data.count(), 2);

        data.clear();

        QVERIFY(data.isEmpty());
        QCOMPARE(data.count(), 0);
    }

    void testEmptyState()
    {
        DetectionData data;
        QVERIFY(data.isEmpty());
        QCOMPARE(data.count(), 0);
        QVERIFY(data.detections().isEmpty());
    }
};

/*******************************************************************************
 * Test Suite: KeypointData Tests
 ******************************************************************************/
class KeypointDataTest : public QObject
{
    Q_OBJECT

private slots:
    /***************************************************************************
     * Construction and Initialization
     **************************************************************************/
    void testDefaultConstruction()
    {
        KeypointData data;
        QVERIFY(data.isEmpty());
        QCOMPARE(data.count(), 0);
    }

    void testConstructionFromVector()
    {
        std::vector<cv::KeyPoint> keypoints;
        keypoints.emplace_back(10.0f, 20.0f, 5.0f);
        keypoints.emplace_back(30.0f, 40.0f, 7.0f);

        KeypointData data(keypoints);

        QVERIFY(!data.isEmpty());
        QCOMPARE(data.count(), 2);
    }

    /***************************************************************************
     * Type ID Verification
     **************************************************************************/
    void testTypeId()
    {
        KeypointData data;
        ::QtNodes::NodeDataType dataType = data.type();

        QCOMPARE(dataType.id, QString("keypoints"));
        QCOMPARE(dataType.name, QString("Keypoints"));
    }

    /***************************************************************************
     * Adding and Retrieving Keypoints
     **************************************************************************/
    void testAddKeypoint()
    {
        KeypointData data;

        cv::KeyPoint kp1(10.0f, 20.0f, 5.0f);
        data.addKeypoint(kp1);

        QCOMPARE(data.count(), 1);
        QVERIFY(!data.isEmpty());

        std::vector<cv::KeyPoint> keypoints = data.keypoints();
        QCOMPARE(keypoints.size(), 1u);
        QCOMPARE(keypoints[0].pt.x, 10.0f);
        QCOMPARE(keypoints[0].pt.y, 20.0f);
        QCOMPARE(keypoints[0].size, 5.0f);
    }

    void testAddMultipleKeypoints()
    {
        KeypointData data;

        for (int i = 0; i < 10; ++i)
        {
            data.addKeypoint(cv::KeyPoint(i * 10.0f, i * 15.0f, 3.0f));
        }

        QCOMPARE(data.count(), 10);
    }

    void testRetrieveKeypoints()
    {
        std::vector<cv::KeyPoint> keypoints;
        keypoints.emplace_back(10.0f, 20.0f, 5.0f, 45.0f, 0.9, 0, -1);
        keypoints.emplace_back(30.0f, 40.0f, 7.0f, 90.0f, 0.8, 1, -1);

        KeypointData data(keypoints);

        std::vector<cv::KeyPoint> retrieved = data.keypoints();
        QCOMPARE(retrieved.size(), 2u);

        QCOMPARE(retrieved[0].pt.x, 10.0f);
        QCOMPARE(retrieved[0].pt.y, 20.0f);
        QCOMPARE(retrieved[0].size, 5.0f);
        QCOMPARE(retrieved[0].angle, 45.0f);

        QCOMPARE(retrieved[1].pt.x, 30.0f);
        QCOMPARE(retrieved[1].pt.y, 40.0f);
        QCOMPARE(retrieved[1].size, 7.0f);
        QCOMPARE(retrieved[1].angle, 90.0f);
    }

    /***************************************************************************
     * Set and Clear Keypoints
     **************************************************************************/
    void testSetKeypoints()
    {
        KeypointData data;

        std::vector<cv::KeyPoint> keypoints1;
        keypoints1.emplace_back(10.0f, 20.0f, 5.0f);
        data.setKeypoints(keypoints1);
        QCOMPARE(data.count(), 1);

        std::vector<cv::KeyPoint> keypoints2;
        keypoints2.emplace_back(30.0f, 40.0f, 7.0f);
        keypoints2.emplace_back(50.0f, 60.0f, 9.0f);
        data.setKeypoints(keypoints2);
        QCOMPARE(data.count(), 2);
    }

    void testClear()
    {
        std::vector<cv::KeyPoint> keypoints;
        keypoints.emplace_back(10.0f, 20.0f, 5.0f);
        keypoints.emplace_back(30.0f, 40.0f, 7.0f);

        KeypointData data(keypoints);
        QCOMPARE(data.count(), 2);

        data.clear();

        QVERIFY(data.isEmpty());
        QCOMPARE(data.count(), 0);
    }

    void testEmptyState()
    {
        KeypointData data;
        QVERIFY(data.isEmpty());
        QCOMPARE(data.count(), 0);
        QVERIFY(data.keypoints().empty());
    }
};

/*******************************************************************************
 * Main Test Runner
 ******************************************************************************/
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("VisionBox Data Types Test");

    int result = 0;

    {
        ImageDataTest imageDataTest;
        result |= QTest::qExec(&imageDataTest, argc, argv);
    }

    {
        DetectionDataTest detectionDataTest;
        result |= QTest::qExec(&detectionDataTest, argc, argv);
    }

    {
        KeypointDataTest keypointDataTest;
        result |= QTest::qExec(&keypointDataTest, argc, argv);
    }

    return result;
}

#include "DataTypesTest.moc"
