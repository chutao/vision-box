# VisionBox - Phase 13: Extended Source Nodes

## Status: ✅ COMPLETED (100%)

### Overview
Phase 13 adds extended source node capabilities to VisionBox. This phase provides essential tools for video file loading, camera capture, and synthetic image generation - fundamental data sources for computer vision workflows.

### Plugins Enhanced

#### 1. ImageSourcePlugin (Enhanced)
**Location:** `plugins/sources/ImageSourcePlugin/`

**Node Models:**
1. **ImageLoaderModel** - Load images from file (existing)
2. **VideoLoaderModel** - Load video files and provide frames (NEW)
3. **CameraSourceModel** - Capture frames from camera (NEW)
4. **ImageGeneratorModel** - Generate synthetic test patterns (NEW)

### Node Models Implemented

#### 1. VideoLoaderModel - Video File Loading
**Description:** Load video files and provide frames with playback control

**Features:**
- **Video Format Support**: MP4, AVI, MOV, MKV, WMV, FLV
- **Playback Controls**:
  - Play/Pause button
  - Frame slider for seeking
  - Frame spinbox for precise navigation
- **Frame Information**:
  - Current frame number
  - Total frame count
  - FPS detection
- **Real-time Processing**: Automatically plays video at correct framerate

**Technical Implementation:**
```cpp
// Video capture with OpenCV
cv::VideoCapture m_capture;
m_capture.open(filePath.toStdString());

// Get video properties
m_totalFrames = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_COUNT));
m_fps = m_capture.get(cv::CAP_PROP_FPS));

// Frame playback with QTimer
m_playbackTimer->setInterval(static_cast<int>(1000.0 / m_fps));

// Frame seeking
m_capture.set(cv::CAP_PROP_POS_FRAMES, frameNumber);
m_capture.read(frame);
```

**Use Cases:**
- Video processing pipelines
- Frame-by-frame analysis
- Video effects and filters
- Motion detection
- Video compression testing
- Object tracking in videos

#### 2. CameraSourceModel - Camera Capture
**Description:** Capture frames from webcam or video camera

**Features:**
- **Camera Selection**:
  - Camera ID selection (0-10)
  - Quick camera selection dropdown
- **Resolution Options**:
  - 320x240, 640x480, 800x600
  - 1280x720, 1600x900, 1920x1080
- **Live Capture**: Real-time camera feed at 30 FPS
- **Status Display**: Shows connection state and resolution

**Technical Implementation:**
```cpp
// Camera capture
cv::VideoCapture m_capture;
m_capture.open(m_cameraId);

// Set resolution
m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_width);
m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_height);

// Get actual resolution
int actualWidth = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_WIDTH));
int actualHeight = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_HEIGHT));

// Continuous capture with QTimer
connect(m_captureTimer, &QTimer::timeout, this, &CameraSourceModel::captureFrame);
```

**Use Cases:**
- Real-time video processing
- Face detection on webcam
- Motion detection systems
- Augmented reality
- Video conferencing effects
- Surveillance applications
- Interactive computer vision demos

#### 3. ImageGeneratorModel - Synthetic Image Generation
**Description:** Generate test patterns and synthetic images

**Pattern Types:**

1. **Solid Color**: Uniform color images
   - Configurable intensity (0-255)
   - Optional random colors
   - Supports grayscale and BGR

2. **Gradient (Horizontal)**: Left-to-right color gradient
   - Configurable start/end intensity
   - Smooth interpolation

3. **Gradient (Vertical)**: Top-to-bottom color gradient
   - Configurable start/end intensity
   - Smooth interpolation

4. **Gradient (Diagonal)**: Corner-to-corner gradient
   - Configurable start/end intensity
   - Smooth interpolation

5. **Checkerboard**: Alternating square pattern
   - Configurable square size
   - Black and white pattern

6. **Grid**: Grid lines pattern
   - Configurable grid spacing
   - Configurable line thickness

7. **Circles**: Random circles
   - Configurable count and radius
   - Optional random colors

8. **Rectangles**: Random rectangles
   - Configurable count and max size
   - Optional random colors

9. **Lines**: Random lines
   - Configurable count and thickness
   - Optional random colors

10. **Gaussian Noise**: Random noise with Gaussian distribution
    - Configurable mean and standard deviation
    - Useful for testing denoising algorithms

11. **Uniform Noise**: Random noise with uniform distribution
    - Configurable min/max range
    - Useful for testing noise robustness

**Technical Implementation:**
```cpp
// Pattern generation
cv::Mat image(m_height, m_width, type);

// Solid color
image.setTo(cv::Scalar(value));

// Gradient
double t = static_cast<double>(x) / (m_width - 1);
double value = startVal + t * (endVal - startVal);

// Checkerboard
bool whiteSquare = ((x / squareSize) + (y / squareSize)) % 2 == 0;

// Random shapes
cv::Point center(
    QRandomGenerator::global()->bounded(m_width),
    QRandomGenerator::global()->bounded(m_height)
);
cv::circle(image, center, radius, color, -1);

// Noise
cv::Mat noise(m_height, m_width, type);
cv::randn(noise, cv::Scalar::all(mean), cv::Scalar::all(stddev));
```

**Configurable Parameters:**
- Image dimensions (width: 64-4096, height: 64-4096)
- Color channels (Grayscale, BGR, RGBA)
- Pattern-specific parameters (count, size, intensity, etc.)
- Random color option

**Use Cases:**
- Algorithm testing and validation
- Generate test datasets
- Pattern recognition testing
- Noise reduction testing
- Edge detection testing
- Calibration patterns
- Synthetic data generation
- Teaching and demonstrations

### Build Configuration

**CMakeLists.txt Updates:**
```cmake
# ImageSourcePlugin (Phase 2 + Phase 13)
add_library(ImageSourcePlugin SHARED
    plugins/sources/ImageSourcePlugin/ImageSourcePlugin.cpp
    plugins/sources/ImageSourcePlugin/ImageLoaderModel.cpp
    plugins/sources/ImageSourcePlugin/VideoLoaderModel.cpp      # Phase 13
    plugins/sources/ImageSourcePlugin/CameraSourceModel.cpp    # Phase 13
    plugins/sources/ImageSourcePlugin/ImageGeneratorModel.cpp  # Phase 13
)
```

### Plugin Loading Verification

**Plugin Count:**
- Total plugins: **23** (no new plugins, just enhanced existing)
- Total node models: **43** (added 3 new models)

**ImageSourcePlugin Models:**
1. ImageLoaderModel (Phase 2)
2. VideoLoaderModel (Phase 13) ✅
3. CameraSourceModel (Phase 13) ✅
4. ImageGeneratorModel (Phase 13) ✅

### Binary Sizes
- ImageSourcePlugin.so: **2.07 MB** (increased from 1.88 MB)

### Technical Highlights

#### Video Processing Architecture
- **OpenCV VideoCapture**: Cross-platform video I/O
- **Frame-based Processing**: Individual frame access for precise control
- **Timer-based Playback**: Accurate framerate simulation
- **Seeking Capability**: Random access to any frame

#### Camera Integration
- **Multiple Camera Support**: Camera ID selection for multi-camera setups
- **Resolution Configuration**: Automatic and manual resolution control
- **Real-time Processing**: 30 FPS continuous capture
- **State Management**: Open/close camera with proper resource cleanup

#### Synthetic Generation
- **Procedural Generation**: Algorithmic pattern creation
- **Randomization**: Pseudo-random test data generation
- **Configurable Parameters**: Extensive customization options
- **Multiple Patterns**: 11 different pattern types

### File Structure

```
plugins/sources/ImageSourcePlugin/
├── ImageSourcePlugin.h
├── ImageSourcePlugin.cpp
├── ImageLoaderModel.h
├── ImageLoaderModel.cpp
├── VideoLoaderModel.h           # Phase 13
├── VideoLoaderModel.cpp         # Phase 13
├── CameraSourceModel.h          # Phase 13
├── CameraSourceModel.cpp        # Phase 13
├── ImageGeneratorModel.h        # Phase 13
├── ImageGeneratorModel.cpp      # Phase 13
└── metadata.json
```

### OpenCV Functions Used

**Video Loading:**
- `cv::VideoCapture::open()` - Open video file
- `cv::VideoCapture::read()` - Read next frame
- `cv::VideoCapture::get()` - Get video properties (CAP_PROP_FRAME_COUNT, CAP_PROP_FPS)
- `cv::VideoCapture::set()` - Set position (CAP_PROP_POS_FRAMES, CAP_PROP_FRAME_WIDTH/HEIGHT)

**Camera Capture:**
- `cv::VideoCapture::open(int deviceID)` - Open camera device
- `cv::VideoCapture::isOpened()` - Check connection status
- `cv::VideoCapture::release()` - Release camera resource

**Image Generation:**
- `cv::Mat::zeros()`, `cv::Mat::ones()` - Initialize matrices
- `cv::Mat::setTo()` - Fill with scalar value
- `cv::randn()` - Gaussian random number generation
- `cv::randu()` - Uniform random number generation
- `cv::circle()` - Draw filled circles
- `cv::rectangle()` - Draw filled rectangles
- `cv::line()` - Draw lines

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
| Phase 9 | 2 | 2 | 34 |
| Phase 10 | 2 | 2 | 36 |
| Phase 11 | 2 | 2 | 38 |
| Phase 12 | 2 | 2 | 40 |
| Phase 13 | 0 | 3 | **43** |
| **Total** | **23** | **43** | **43** |

### Key Achievements

✅ Video file loading with playback controls
✅ Camera capture with multi-camera support
✅ 11 synthetic image generation patterns
✅ Real-time video processing capability
✅ Resolution control for cameras and videos
✅ Frame seeking in videos
✅ Test pattern generation for algorithm validation
✅ Random shape generation
✅ Noise pattern generation (Gaussian and uniform)
✅ 3 new node models in Phase 13
✅ All 23 plugins load successfully
✅ Enhanced source node capabilities

### Use Cases by Model

**VideoLoaderModel:**
- Video processing pipelines
- Frame-by-frame analysis
- Video effects and filters
- Motion detection
- Object tracking in videos
- Video compression testing
- Video stabilization testing
- Background subtraction
- Video annotation
- Video format conversion

**CameraSourceModel:**
- Real-time video processing
- Face detection on webcam
- Motion detection systems
- Augmented reality
- Video conferencing effects
- Surveillance applications
- Interactive demos
- Gesture recognition
- Pose estimation
- QR/barcode scanning

**ImageGeneratorModel:**
- Algorithm testing and validation
- Generate test datasets
- Pattern recognition testing
- Noise reduction testing
- Edge detection testing
- Calibration patterns
- Synthetic data generation
- Teaching and demonstrations
- Unit test fixtures
- Benchmarking

### Dependencies

No new dependencies were added in Phase 13. All features use existing:
- Qt6 (Core, Widgets, GUI)
- OpenCV 4.6.0 (core, imgproc, imgcodecs, videoio)
- QtNodes 3.0.12

**Note:** Phase 13 uses existing OpenCV modules:
- `videoio` for video and camera capture
- `imgproc` for image generation operations

### Testing

**Build Test:**
```bash
cmake ..
make -j$(nproc)
```
✅ All 23 plugins compile without errors

**Plugin Loading Test:**
```bash
./VisionBox
```
✅ All 23 plugins load successfully

**Binary Sizes:**
- ImageSourcePlugin.so: 2.07 MB (increased from 1.88 MB)

### Completion Date
January 27, 2026

---

**Phase 13 Status: ✅ COMPLETE**

VisionBox now includes comprehensive source node capabilities with video loading, camera capture, and synthetic image generation. These tools enable researchers to work with real-time video feeds, process video files, and generate test data for algorithm validation - essential capabilities for any computer vision research framework.

**Total: 43 node models across 23 plugins**
