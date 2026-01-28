# VisionBox - Phase 9: Video Processing and Object Detection

## Status: ✅ COMPLETED (100%)

### Overview
Phase 9 adds video processing and object detection capabilities to VisionBox. This phase provides essential tools for video analysis, motion detection, and object recognition - fundamental techniques for surveillance, analytics, and automated monitoring applications.

### Plugins Implemented

#### 1. Video Processing Plugin (`VideoProcessingPlugin`)
**Location:** `plugins/video/VideoProcessingPlugin/`

**Node Models:**
- **BackgroundSubtractionModel** - Separate foreground from background in video streams
  - **MOG2 (Mixture of Gaussians)**: Adaptive background modeling
    - Based on Gaussian mixture models
    - Handles repetitive background motions
    - Adapts to gradual illumination changes
    - Uses `cv::createBackgroundSubtractorMOG2()` for computation
  - **KNN (K-Nearest Neighbors)**: KNN-based background subtraction
    - Uses K-nearest neighbors clustering
    - Faster than MOG2
    - Good for static backgrounds
    - Uses `cv::createBackgroundSubtractorKNN()` for computation
  - Configurable parameters:
    - History (10-1000): Number of frames for background model
    - Threshold (0.0-100.0): Threshold for foreground detection
    - Detect shadows: Enable shadow detection (marked as gray)
    - Learning rate (-1.0-1.0): Background adaptation rate
  - Visual feedback: Highlights foreground in green, shadows in darkened pixels
  - Reset button to reinitialize background model

**Technical Implementation:**
```cpp
// MOG2 Background Subtraction:
cv::Ptr<cv::BackgroundSubtractorMOG2> mog2 =
    cv::createBackgroundSubtractorMOG2(history, varThreshold, detectShadows);
mog2->apply(frame, fgMask, learningRate);
// Foreground mask: 0=background, 255=foreground, 127=shadow

// KNN Background Subtraction:
cv::Ptr<cv::BackgroundSubtractorKNN> knn =
    cv::createBackgroundSubtractorKNN(history, varThreshold, detectShadows);
knn->apply(frame, fgMask, learningRate);
```

**Background Subtraction Applications:**
- Video surveillance and security
- Motion detection and tracking
- People counting
- Traffic monitoring
- Indoor/outdoor intrusion detection
- Automated video analytics
- Background removal for video processing
- Action recognition preprocessing

#### 2. Object Detection Plugin (`ObjectDetectionPlugin`)
**Location:** `plugins/detection/ObjectDetectionPlugin/`

**Node Models:**
- **HOGDetectionModel** - Detect pedestrians using HOG descriptors
  - **Histogram of Oriented Gradients (HOG)**: Feature-based object detection
    - Uses gradient orientation histograms
    - SVM-based classifier
    - Default people detector (Dalal-Triggs detector)
    - Scale-invariant multi-scale detection
    - Uses `cv::HOGDescriptor` for computation
  - Configurable parameters:
    - Hit threshold (-10.0 to 10.0): Detection threshold (lower = more detections)
    - Window stride (4-32): Sliding window step size
    - Padding (0-32): Padding around detection window
    - Scale factor (1.01-2.0): Image pyramid scale factor
    - Mean shift grouping: Merge overlapping detections
    - Draw bounding boxes: Visualize detected objects
  - Visual feedback: Draws green bounding boxes with labels

**Technical Implementation:**
```cpp
// HOG-based Pedestrian Detection:
cv::HOGDescriptor hog;
hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

std::vector<cv::Rect> foundLocations;
std::vector<double> weights;

hog.detectMultiScale(image, foundLocations, weights,
                    hitThreshold, winStride, padding,
                    scale, finalThreshold);

// Draw bounding boxes:
cv::rectangle(output, rect, cv::Scalar(0, 255, 0), 2);  // Green
```

**HOG Detection Applications:**
- Pedestrian detection in surveillance
- People counting and tracking
- Autonomous driving and ADAS
- Robotics navigation
- Crowd monitoring
- Safety systems
- Smart transportation
- Access control

### Build Configuration

**CMakeLists.txt Updates:**
```cmake
# Added objdetect module for HOG detection
find_package(OpenCV 4.0 REQUIRED COMPONENTS
    core
    imgproc
    imgcodecs
    videoio
    objdetect
)

# VideoProcessingPlugin (Phase 9)
add_library(VideoProcessingPlugin SHARED
    plugins/video/VideoProcessingPlugin/VideoProcessingPlugin.cpp
    plugins/video/VideoProcessingPlugin/BackgroundSubtractionModel.cpp
)

# ObjectDetectionPlugin (Phase 9)
add_library(ObjectDetectionPlugin SHARED
    plugins/detection/ObjectDetectionPlugin/ObjectDetectionPlugin.cpp
    plugins/detection/ObjectDetectionPlugin/HOGDetectionModel.cpp
)
```

### Plugin Loading Verification

**Test Results:**
```
Loaded plugins: 17
  - "Basic Filter Plugin" v "1.0.0"           (Phase 2)
  - "Color Adjustment Plugin" v "1.0.0"       (Phase 4)
  - "Contour Plugin" v "1.0.0"                (Phase 6)
  - "Edge Detection Plugin" v "1.0.0"         (Phase 3)
  - "Image Enhancement Plugin" v "1.0.0"      (Phase 5)
  - "Feature Descriptors Plugin" v "1.0.0"    (Phase 8)
  - "Feature Detection Plugin" v "1.0.0"      (Phase 7)
  - "Geometric Transform Plugin" v "1.0.0"    (Phase 3)
  - "Histogram Plugin" v "1.0.0"              (Phase 5)
  - "Image Arithmetic Plugin" v "1.0.0"       (Phase 4)
  - "Image Source Plugin" v "1.0.0"           (Phase 2)
  - "Image Viewer Plugin" v "1.0.0"           (Phase 2)
  - "Object Detection Plugin" v "1.0.0"       (Phase 9) ✅
  - "Optical Flow Plugin" v "1.0.0"           (Phase 8)
  - "Segmentation Plugin" v "1.0.0"            (Phase 6)
  - "Template Matching Plugin" v "1.0.0"       (Phase 7)
  - "Video Processing Plugin" v "1.0.0"       (Phase 9) ✅
```

### Technical Highlights

#### Background Subtraction
- **MOG2 Algorithm**:
  - Mixture of Gaussians models each pixel as mixture of K Gaussians
  - Adaptive learning: background model updates over time
  - Shadow detection: distinguishes shadows from foreground
  - Good for: outdoor scenes, varying lighting, dynamic backgrounds
  - Computational complexity: O(W×H×K) where W,H = image dimensions

- **KNN Algorithm**:
  - K-nearest neighbors classification for background/foreground
  - Faster than MOG2
  - Simpler model
  - Good for: indoor scenes, static backgrounds
  - Computational complexity: O(W×H×K) but with smaller constant

**Parameter Guide:**
- **History**: Number of frames for background model
  - Larger = more stable but slower to adapt
  - Typical: 200-500 frames
- **Threshold**: Foreground detection threshold
  - Lower = more sensitive (more false positives)
  - Higher = less sensitive (more false negatives)
  - MOG2: 16 default, KNN: 400 default
- **Detect Shadows**: Enable shadow detection
  - Shadows marked as 127 (gray) in mask
  - Helps avoid shadow triggering motion detection
- **Learning Rate**: Background model adaptation speed
  - -1 = automatic (default)
  - 0 = fixed background (no learning)
  - 1 = rapid learning

#### HOG Object Detection
- **HOG Features**:
  - Divide image into small spatial cells
  - Compute gradient orientations for each cell
  - Create histogram of orientations for each cell
  - Normalize histograms across blocks
  - Train SVM classifier on features

- **Multi-scale Detection**:
  - Build image pyramid (scaled versions)
  - Apply sliding window at each scale
  - Compute HOG features for each window
  - Classify using SVM
  - Merge overlapping detections

**Parameter Guide:**
- **Hit Threshold**: SVM detection threshold
  - Lower = more detections (more false positives)
  - Higher = fewer detections (more false negatives)
  - Typical: 0.0 (default) to 0.5
- **Window Stride**: Sliding window step
  - Smaller = more detections (slower)
  - Larger = fewer detections (faster)
  - Must be multiple of block size (8 pixels)
  - Typical: 8 pixels
- **Padding**: Detection window padding
  - Adds padding around detection area
  - Helps detect objects near borders
  - Typical: 8-16 pixels
- **Scale Factor**: Pyramid scale between levels
  - Smaller = more scales (slower, more accurate)
  - 1.05 = 5% size reduction per level (default)
  - Typical: 1.01-1.1

### File Structure

```
plugins/
├── video/
│   └── VideoProcessingPlugin/
│       ├── VideoProcessingPlugin.h
│       ├── VideoProcessingPlugin.cpp
│       ├── BackgroundSubtractionModel.h
│       ├── BackgroundSubtractionModel.cpp
│       └── metadata.json
└── detection/
    └── ObjectDetectionPlugin/
        ├── ObjectDetectionPlugin.h
        ├── ObjectDetectionPlugin.cpp
        ├── HOGDetectionModel.h
        ├── HOGDetectionModel.cpp
        └── metadata.json
```

### OpenCV Functions Used

**Video Processing:**
- `cv::createBackgroundSubtractorMOG2()` - Create MOG2 background subtractor
- `cv::createBackgroundSubtractorKNN()` - Create KNN background subtractor
- `cv::BackgroundSubtractor::apply()` - Apply background subtraction
- `cv::BackgroundSubtractorMOG2` - MOG2 background subtractor class
- `cv::BackgroundSubtractorKNN` - KNN background subtractor class

**Object Detection:**
- `cv::HOGDescriptor` - HOG descriptor class
- `cv::HOGDescriptor::getDefaultPeopleDetector()` - Get default people detector
- `cv::HOGDescriptor::setSVMDetector()` - Set SVM detector
- `cv::HOGDescriptor::detectMultiScale()` - Multi-scale object detection
- `cv::rectangle()` - Draw bounding boxes
- `cv::putText()` - Add labels

### Node Model Count Progress

| Phase | Plugins | Node Models | Total |
|-------|---------|-------------|-------|
| Phase 1 | 0 | 0 | 0 |
| Phase 2 | 3 | 7 | 7 |
| Phase 3 | 2 | 7 | 14 |
| Phase 4 | 2 | 8 | 22 |
| Phase 5 | 2 | 4 | 26 |
| Phase 6 | 2 | 2 | 28 |
| Phase 7 | 2 | 2 | 30 |
| Phase 8 | 2 | 2 | 32 |
| Phase 9 | 2 | 2 | **34** |
| **Total** | **17** | **34** | **34** |

### Key Achievements

✅ Background subtraction with MOG2 and KNN algorithms
✅ HOG-based pedestrian detection
✅ Motion detection and video analytics
✅ Adaptive background modeling
✅ Multi-scale object detection
✅ Shadow detection in background subtraction
✅ Foreground visualization with color coding
✅ Bounding box visualization with labels
✅ 2 new node models in Phase 9
✅ All 17 plugins load successfully
✅ Video processing capabilities
✅ Object detection for surveillance

### Plugin Summary

**Phase 9 Implementation:**
1. **BackgroundSubtractionModel** - MOG2 and KNN background subtraction
2. **HOGDetectionModel** - HOG-based pedestrian detection

**Total Node Models by Plugin:**
- ImageSourcePlugin: 1 (ImageLoader)
- BasicFilterPlugin: 4 (Blur, Threshold, Morphology, ColorConvert)
- ImageViewerPlugin: 1 (ImageViewer)
- EdgeDetectionPlugin: 4 (Canny, Sobel, Laplacian, Scharr)
- GeometricTransformPlugin: 3 (Resize, Rotate, Flip)
- ImageArithmeticPlugin: 6 (Add, Subtract, Multiply, Divide, AbsDiff, Blend)
- ColorAdjustmentPlugin: 2 (Brightness/Contrast, Saturation)
- HistogramPlugin: 1 (Histogram Equalization)
- EnhancementPlugin: 2 (Denoise, Sharpen)
- SegmentationPlugin: 1 (K-Means)
- ContourPlugin: 1 (Contour Finder)
- FeatureDetectionPlugin: 1 (Corner Detection)
- TemplateMatchingPlugin: 1 (Template Matching)
- OpticalFlowPlugin: 1 (Optical Flow)
- FeatureDescriptorsPlugin: 1 (ORB Features)
- **VideoProcessingPlugin: 1 (Background Subtraction)** ⬆️
- **ObjectDetectionPlugin: 1 (HOG Detection)** ⬆️

### Use Cases

**Background Subtraction:**
- Video surveillance and security systems
- Motion-triggered recording
- Intrusion detection
- People counting in retail
- Traffic monitoring
- Wildlife monitoring
- Industrial automation
- Human-computer interaction
- Video conferencing (virtual backgrounds)
- Action recognition preprocessing

**HOG Object Detection:**
- Pedestrian detection in autonomous vehicles
- People counting for crowd analysis
- Surveillance and security monitoring
- Advanced driver assistance systems (ADAS)
- Robotics and autonomous navigation
- Smart traffic systems
- Retail analytics
- Safety monitoring systems
- Access control
- Search and rescue operations

### Dependencies

Phase 9 added one new OpenCV module:
- Qt6 (Core, Widgets)
- OpenCV 4.6.0 (core, imgproc, imgcodecs, videoio, photo, calib3d, video, features2d, **objdetect**)
- QtNodes 3.0.12

**Note:** Phase 9 uses OpenCV `objdetect` module for `cv::HOGDescriptor`.

### Testing

**Build Test:**
```bash
cmake ..
make -j$(nproc)
```
✅ All 17 plugins compile without errors

**Plugin Loading Test:**
```bash
./VisionBox
```
✅ All 17 plugins load successfully

**Binary Sizes:**
- VideoProcessingPlugin.so: 1.9 MB (Background Subtraction)
- ObjectDetectionPlugin.so: 1.9 MB (HOG Detection)

### Completion Date
January 27, 2026

---

**Phase 9 Status: ✅ COMPLETE**

VisionBox now includes advanced video processing and object detection capabilities. Background subtraction enables motion detection and video analytics, while HOG-based object detection provides pedestrian detection for surveillance and autonomous systems. These tools are essential for advanced computer vision applications including security monitoring, intelligent transportation, and automated surveillance.

**Total: 34 node models across 17 plugins**
