# VisionBox - Phase 16: DNN Inference and Object Tracking

## Status: ✅ COMPLETED (100%)

### Overview
Phase 16 adds machine learning and advanced tracking capabilities to VisionBox. This phase provides essential tools for deep neural network inference and real-time object tracking - fundamental techniques for modern computer vision applications.

### Plugins Implemented

#### 1. MachineLearningPlugin (NEW)
**Location:** `plugins/ml/MachineLearningPlugin/`

**Node Models:**

##### 1.1 DNNInferenceModel - Deep Neural Network Inference
**Description:** Load and run inference on deep learning models

**Technical Details:**
- **Supported Model Formats**:
  - ONNX (.onnx) - Open Neural Network Exchange
  - TensorFlow (.pb) - Frozen graphs
  - Caffe (.caffemodel with .prototxt) - Caffe models
- **Backend Options**:
  - OpenCV DNN
  - CUDA (GPU acceleration)
  - Inference Engine (Intel)
  - Variance (OpenVINO)
  - CPU (default)
- **Target Options**:
  - CPU
  - CUDA FP32/FP16 (NVIDIA GPU)
  - FPGA
  - Vulkan
  - OpenCL/OpenCL FP16

**Configurable Parameters:**
- **Model File**: Path to model weights
- **Config File**: Path to model configuration (for Caffe/TensorFlow)
- **Classes File**: Text file with class names (one per line)
- **Backend Selection**: Computation backend
- **Target Selection**: Device target (CPU/GPU/FPGA)
- **Confidence Threshold (0.0-1.0)**: Detection confidence threshold (default: 0.5)
- **NMS Threshold (0.0-1.0)**: Non-maximum suppression (default: 0.4)
- **Swap RB**: Swap Red and Blue channels (BGR to RGB conversion)
- **Auto-loading**: Model loads automatically when restored from file

**Technical Implementation:**
```cpp
// Load ONNX model
cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);

// Set backend and target
net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

// Preprocess image
cv::Mat blob = cv::dnn::blobFromImage(image, scale, inputSize, mean,
                                       swapRB, false, CV_32F);

// Run inference
net.setInput(blob);
std::vector<cv::Mat> outs;
net.forward(outs);
```

**Workflow:**
1. Select model file (.onnx, .pb, .caffemodel)
2. (Optional) Select config file for Caffe/TensorFlow
3. (Optional) Select classes file for label mapping
4. Click "Load Model" to load the network
5. Adjust confidence and NMS thresholds
6. Input image is automatically processed through the network
7. Output shows detected/annotated results

**Applications:**
- Object detection (YOLO, SSD, Faster R-CNN)
- Image classification (ResNet, MobileNet, EfficientNet)
- Semantic segmentation (DeepLab, PSPNet)
- Pose estimation (OpenPose, MediaPipe)
- Face recognition (ArcFace, FaceNet)
- Custom deep learning models
- Real-time inference with GPU acceleration
- Model deployment and testing

##### 1.2 ObjectTrackerModel - Real-Time Object Tracking
**Description:** Track objects in video sequences using various algorithms

**Technical Details:**
- **Motion Detection**: Background subtraction with MOG2
  - Adaptive background model
  - Handles illumination changes
  - Shadow detection
  - Morphological filtering

- **Dense Optical Flow**: Farneback algorithm
  - Computes flow for all pixels
  - Visualization with flow vectors
  - Robust to illumination changes
  - Uses `cv::calcOpticalFlowFarneback()`

- **Sparse Optical Flow**: Lucas-Kanade pyramidal
  - Tracks feature points
  - Faster computation
  - Good for specific feature tracking
  - Uses `cv::calcOpticalFlowPyrLK()`

**Configurable Parameters:**
- **Tracker Type**: Motion Detection / Dense OF / Sparse OF
- **Threshold (0-255)**: Motion detection sensitivity (default: 25)
  - Lower = more sensitive (more motion detected)
  - Higher = less sensitive (only large motions)
- **Morphology**: Apply opening/closing to clean noise
- **Min Area (1-10000)**: Minimum object area in pixels
- **Max Area (100-1000000)**: Maximum object area in pixels
- **Show Bounding Boxes**: Visualize detected/tracked objects

**Technical Implementation:**

**Motion Detection:**
```cpp
// Background subtraction
cv::Ptr<cv::BackgroundSubtractorMOG2> bgSubtractor =
    cv::createBackgroundSubtractorMOG2();

cv::Mat fgMask;
bgSubtractor->apply(gray, fgMask);
cv::threshold(fgMask, fgMask, threshold, 255, cv::THRESH_BINARY);

// Morphological operations
cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);

// Find contours
std::vector<std::vector<cv::Point>> contours;
cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

// Filter by area and draw bounding boxes
for (auto& contour : contours) {
    double area = cv::contourArea(contour);
    if (area >= minArea && area <= maxArea) {
        cv::Rect bbox = cv::boundingRect(contour);
        cv::rectangle(result, bbox, cv::Scalar(0, 255, 0), 2);
    }
}
```

**Dense Optical Flow:**
```cpp
cv::Mat flow;
cv::calcOpticalFlowFarneback(prevFrame, currentFrame, flow,
                              0.5,  // pyr_scale
                              3,    // pyr_levels
                              15,   // winsize
                              3,    // iterations
                              5,    // poly_n
                              1.2,  // poly_sigma
                              0);   // flags

// Visualize flow vectors
for (int y = 0; y < flow.rows; y += 10) {
    for (int x = 0; x < flow.cols; x += 10) {
        cv::Point2f fxy = flow.at<cv::Point2f>(y, x);
        cv::line(result, cv::Point(x, y),
                cv::Point(x + fxy.x, y + fxy.y),
                cv::Scalar(0, 255, 0), 1);
    }
}
```

**Sparse Optical Flow:**
```cpp
// Detect good features to track
std::vector<cv::Point2f> prevPoints;
cv::goodFeaturesToTrack(gray, prevPoints, 100,
                       0.01, 10, cv::Mat(), 3, false, 0.04);

// Track across frames
std::vector<cv::Point2f> currPoints;
std::vector<uchar> status;
std::vector<float> err;
cv::calcOpticalFlowPyrLK(prevFrame, currFrame,
                          prevPoints, currPoints,
                          status, err);

// Visualize tracking
for (size_t i = 0; i < currPoints.size(); ++i) {
    if (status[i]) {
        cv::circle(result, prevPoints[i], 3, cv::Scalar(0, 255, 0), -1);
        cv::circle(result, currPoints[i], 3, cv::Scalar(0, 0, 255), -1);
        cv::line(result, prevPoints[i], currPoints[i], cv::Scalar(255, 0, 0), 1);
    }
}
```

**Applications:**
- **Motion Detection**:
  - Video surveillance
  - Security systems
  - Intrusion detection
  - People counting
  - Traffic monitoring

- **Optical Flow**:
  - Motion analysis
  - Video stabilization
  - Action recognition
  - Speed estimation
  - Object motion segmentation
  - Frame interpolation

- **Tracking**:
  - Single object tracking
  - Multi-object tracking
  - Feature point tracking
  - Visual odometry
  - SLAM (Simultaneous Localization and Mapping)
  - Augmented reality

### Build Configuration

**CMakeLists.txt Updates:**
```cmake
# MachineLearningPlugin (Phase 16)
option(BUILD_MACHINELEARNING_PLUGIN "Build MachineLearningPlugin" ON)
add_library(MachineLearningPlugin SHARED
    plugins/ml/MachineLearningPlugin/MachineLearningPlugin.cpp
    plugins/ml/MachineLearningPlugin/DNNInferenceModel.cpp
    plugins/ml/MachineLearningPlugin/ObjectTrackerModel.cpp
)

target_link_libraries(MachineLearningPlugin PRIVATE
    ${QT_LIBRARIES}
    QtNodes::QtNodes
    ${OpenCV_LIBS}  # Must include dnn module
)
```

### Plugin Loading Verification

**Test Results:**
```
Total plugins: 26 (Phase 16 added 1 new plugin)
Total node models: 50 (Phase 16 added 2 new models)
```

**Plugin List:**
- MachineLearningPlugin (Phase 16) ✅
  - DNN Inference
  - Object Tracker

**Binary Sizes:**
- MachineLearningPlugin.so: **2.21 MB**

### Technical Highlights

#### DNN Module Integration
- **OpenCV DNN Module**: `cv::dnn` namespace
  - No external framework dependencies
  - Cross-platform inference
  - Multiple format support
  - GPU acceleration via CUDA
  - CPU fallback compatibility

#### Model Format Support

| Format | Extension | Config File | Use Case |
|--------|-----------|-------------|----------|
| ONNX | .onnx | Not needed | PyTorch, TensorFlow export |
| TensorFlow | .pb | .pbtxt (optional) | TensorFlow frozen graphs |
| Caffe | .caffemodel | .prototxt | Caffe models |

#### Optical Flow Comparison

| Method | Type | Speed | Accuracy | Use Case |
|--------|------|-------|----------|----------|
| Farneback | Dense | Medium | High | General motion analysis |
| Lucas-Kanade | Sparse | Fast | Medium | Feature tracking |
| Background Subtraction | - | Fastest | Low | Simple motion detection |

#### Tracker Comparison

| Tracker | Speed | Robustness | Precision | Use Case |
|---------|-------|------------|----------|----------|
| Motion Detection | ⭐⭐⭐ | ⭐⭐ | ⭐ | Surveillance, counting |
| Dense Optical Flow | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ | Motion analysis, stabilization |
| Sparse Optical Flow | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | Feature tracking, SLAM |

### File Structure

```
plugins/ml/MachineLearningPlugin/
├── MachineLearningPlugin.h
├── MachineLearningPlugin.cpp
├── DNNInferenceModel.h
├── DNNInferenceModel.cpp
├── ObjectTrackerModel.h
├── ObjectTrackerModel.cpp
└── metadata.json
```

### OpenCV Functions Used

**DNN Inference:**
- `cv::dnn::readNetFromONNX()` - Load ONNX models
- `cv::dnn::readNetFromTensorflow()` - Load TensorFlow models
- `cv::dnn::readNetFromCaffe()` - Load Caffe models
- `cv::dnn::blobFromImage()` - Preprocess image to blob
- `cv::dnn::Net::setInput()` - Set input to network
- `cv::dnn::Net::forward()` - Run inference
- `cv::dnn::Net::setPreferableBackend()` - Set computation backend
- `cv::dnn::Net::setPreferableTarget()` - Set device target

**Object Tracking:**
- `cv::createBackgroundSubtractorMOG2()` - Create MOG2 background subtractor
- `cv::BackgroundSubtractorMOG2::apply()` - Update background model
- `cv::calcOpticalFlowFarneback()` - Dense optical flow
- `cv::calcOpticalFlowPyrLK()` - Sparse optical flow
- `cv::goodFeaturesToTrack()` - Detect feature points
- `cv::morphologyEx()` - Morphological operations
- `cv::findContours()` - Find object contours
- `cv::boundingRect()` - Get bounding rectangles

### Node Model Count Progress

| Phase | Plugins | Node Models | Total |
|-------|---------|-------------|-------|
| Phase 1-14 | 24 | 46 | 46 |
| Phase 15 | 1 | 2 | 48 |
| Phase 16 | 1 | 2 | **50** |
| **Total** | **26** | **50** | **50** |

### Key Achievements

✅ DNN model loading (ONNX, TensorFlow, Caffe)
✅ Multiple backend support (OpenCV, CUDA, Inference Engine)
✅ GPU acceleration support (CUDA FP16/FP32)
✅ Configurable preprocessing (scale, mean, swap RB)
✅ Class name file support
✅ Confidence and NMS thresholding
✅ Real-time object tracking
✅ Motion detection with background subtraction
✅ Dense optical flow visualization
✅ Sparse optical flow with feature tracking
✅ Morphological noise filtering
✅ Bounding box visualization
✅ 2 new node models in Phase 16
✅ All 26 plugins load successfully
✅ **50 node models milestone achieved!**

### Use Cases by Model

**DNNInferenceModel:**
- Object detection (YOLO, SSD, Faster R-CNN)
- Image classification (ResNet, MobileNet, EfficientNet)
- Semantic segmentation (DeepLab, PSPNet, U-Net)
- Instance segmentation (Mask R-CNN)
- Pose estimation (OpenPose, MoveNet, BlazePose)
- Face detection (RetinaFace, MTCNN, UltraLight)
- Face recognition (ArcFace, FaceNet, CosFace)
- Depth estimation (MiDaS, DPT)
- Super resolution (ESPCN, SRGAN)
- Style transfer (Fast Style Transfer, AdaIN)
- Custom model deployment
- Model testing and validation
- Batch processing
- Real-time inference with GPU

**ObjectTrackerModel:**
- Video surveillance systems
- Intrusion detection
- People counting
- Vehicle tracking
- Traffic monitoring
- Sports analytics
- Motion capture
- Video stabilization
- Action recognition
- Gesture recognition
- Visual SLAM
- Augmented reality
- Robot navigation
- Automated camera tracking
- Wildlife monitoring
- Crowd analysis

### Dependencies

No new dependencies were added in Phase 16. All features use existing:
- Qt6 (Core, Widgets)
- OpenCV 4.6.0 (core, imgproc, imgcodecs, videoio, **dnn**, video)
- QtNodes 3.0.12

**Note:** Phase 16 uses OpenCV's DNN module:
- `dnn` for deep neural network inference
- `video` for optical flow and tracking

### DNN Backend Requirements

**CPU (Default):**
- Always available
- Works on all systems
- Suitable for small models and batch processing

**CUDA (NVIDIA GPU):**
- Requires CUDA toolkit
- Requires OpenCV built with CUDA support
- Significant speedup for inference
- FP16 support for faster computation

**Inference Engine (Intel):**
- Requires OpenVINO toolkit
- Optimized for Intel hardware
- Best for Intel CPUs/GPUs/VPUs

### Testing

**Build Test:**
```bash
cmake ..
make -j$(nproc)
```
✅ All 26 plugins compile without errors

**Plugin Loading Test:**
```bash
./VisionBox
```
✅ All 26 plugins load successfully

**Binary Sizes:**
- MachineLearningPlugin.so: 2.21 MB

### Completion Date
January 27, 2026

---

**Phase 16 Status: ✅ COMPLETE**

VisionBox now includes machine learning and object tracking capabilities. DNN inference enables deployment of state-of-the-art deep learning models for object detection, classification, and segmentation. Object tracking provides real-time motion analysis and feature tracking capabilities. These tools are essential for modern computer vision applications including surveillance, robotics, augmented reality, and automated systems.

**🎉 Milestone: 50 node models across 26 plugins!**

**Next Phase Options:**
- Phase 17: Export functionality (image/video/data export)
- Phase 18: Advanced detectors (YOLO, GrabCut)
- Phase 19: Visualization enhancements (bounding boxes, keypoint viewers)
- Phase 20: Documentation and testing
