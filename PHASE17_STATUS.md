# VisionBox - Phase 17: Export Functionality

## Status: ✅ COMPLETED (100%)

### Overview
Phase 17 adds essential export functionality to VisionBox. This phase provides tools for saving images, recording videos, and exporting data to various formats - critical capabilities for any computer vision research framework.

### Plugins Implemented

#### 1. ExportPlugin (NEW)
**Location:** `plugins/exporters/ExportPlugin/`

**Node Models:**

##### 1.1 ImageExporterModel - Save Images to Disk
**Description:** Export images to various file formats

**Technical Details:**
- **Supported Formats**:
  - PNG (lossless compression)
  - JPEG (lossy compression)
  - BMP (uncompressed)
  - TIFF (LZW compression)
  - WebP (modern format)

- **Configurable Parameters**:
  - **Output Directory**: Where to save images
  - **File Prefix**: Base filename (e.g., "image", "result")
  - **Format Selection**: PNG/JPEG/BMP/TIFF/WebP
  - **Quality (1-100)**: For JPEG and WebP formats
  - **Auto-Increment Filename**: Add frame numbers (image_001.png, image_002.png, etc.)

**Technical Implementation:**
```cpp
// Export with compression parameters
std::vector<int> compression_params;
compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
compression_params.push_back(m_quality);
cv::imwrite(filePath.toStdString(), image, compression_params);
```

**Workflow:**
1. Select output directory
2. Set file prefix (optional)
3. Choose output format
4. Adjust quality for lossy formats
5. Enable auto-increment for sequential exports
6. Click "Export Image" button to save

**Applications:**
- Saving processed results
- Exporting frames from video processing
- Benchmarking result storage
- Batch processing output
- Dataset creation
- Result archival

##### 1.2 VideoExporterModel - Record Video Streams
**Description:** Record video sequences to disk

**Technical Details:**
- **Supported Formats**:
  - MP4 (H.264 codec) - Most compatible
  - AVI (XVID codec) - Legacy Windows format
  - AVI (MJPG codec) - Motion JPEG
  - AVI (DIVX codec) - DivX format
  - MKV (H.264 codec) - Modern container
  - MOV (H.264 codec) - QuickTime format

- **Configurable Parameters**:
  - **Output File Path**: Full path to output video file
  - **Format**: Container/codec selection
  - **FPS (1-120)**: Frames per second (default: 30)
  - **Quality (1-100)**: Video quality/compression (default: 95)

- **Recording States**:
  - **Idle**: Ready to record
  - **Recording**: Actively capturing frames
  - **Paused**: Recording paused (future enhancement)

**Technical Implementation:**
```cpp
// Initialize video writer with fourcc codec
int fourcc = cv::VideoWriter::fourcc('m','p','4','v');
m_writer.open(m_outputPath.toStdString(),
             fourcc,
             m_fps,
             image.size(),
             true);

// Write frames
m_writer.write(frame);
m_frameCount++;
```

**Workflow:**
1. Select output file path with file browser
2. Choose video format
3. Set FPS (must match source frame rate)
4. Adjust quality settings
5. Connect image source to input port
6. Click "Start Recording" to begin
7. Click "Stop Recording" to finalize video

**Applications:**
- Recording processed video output
- Creating demo videos
- Saving analysis results
- Video format conversion
- Real-time recording
- Surveillance recording
- Motion capture recording

##### 1.3 DataExporterModel - Export Data to CSV/JSON
**Description:** Export numerical and statistical data to structured formats

**Technical Details:**
- **Export Formats**:
  - CSV (Comma-Separated Values)
  - JSON (JavaScript Object Notation)

- **Data Types**:
  - **Image Info**: Basic image properties (width, height, channels, depth, min/max values)
  - **Statistics**: Statistical analysis (mean, standard deviation)
  - **Histogram**: Color distribution data
  - **Detection Results**: Future support for detection coordinates

**Collected Data:**
```cpp
struct ImageInfo {
    int width;
    int height;
    int channels;
    int depth;
    double minValue;
    double maxValue;
    double meanValue;
    double stdDev;
};
```

**Configurable Parameters**:
- **Output Directory**: Where to save data files
- **File Prefix**: Base filename
- **Format**: CSV or JSON
- **Data Type**: What data to export
- **Auto-Increment Filename**: Add frame numbers
- **Include Timestamp**: Add date/time to filename

**CSV Output Example:**
```csv
Property,Value
Width,1920
Height,1080
Channels,3
Depth,0
Min Value,0.0000
Max Value,255.0000
Mean,127.456789
Std Dev,65.432109
```

**JSON Output Example:**
```json
{
  "type": "image_info",
  "timestamp": "20250127_143000",
  "data": {
    "width": 1920,
    "height": 1080,
    "channels": 3,
    "depth": 0,
    "min_value": 0.0,
    "max_value": 255.0,
    "mean": 127.456789,
    "std_dev": 65.432109
  }
}
```

**Workflow:**
1. Select output directory
2. Set file prefix
3. Choose export format (CSV/JSON)
4. Select data type to export
5. Enable auto-increment if needed
6. Enable timestamp if needed
7. Connect image source to input
8. Click "Export Data" button

**Applications:**
- Statistical analysis export
- Benchmarking data collection
- Measurement logging
- Result documentation
- Data analysis workflows
- Machine learning dataset creation
- Research data export
- Integration with other tools

### Build Configuration

**CMakeLists.txt Updates:**
```cmake
# ExportPlugin (Phase 17)
option(BUILD_EXPORT_PLUGIN "Build ExportPlugin" ON)
add_library(ExportPlugin SHARED
    plugins/exporters/ExportPlugin/ExportPlugin.cpp
    plugins/exporters/ExportPlugin/ImageExporterModel.cpp
    plugins/exporters/ExportPlugin/VideoExporterModel.cpp
    plugins/exporters/ExportPlugin/DataExporterModel.cpp
)

target_link_libraries(ExportPlugin PRIVATE
    ${QT_LIBRARIES}
    QtNodes::QtNodes
    ${OpenCV_LIBS}
)
```

### Plugin Loading Verification

**Test Results:**
```
Total plugins: 27 (Phase 17 added 1 new plugin)
Total node models: 53 (Phase 17 added 3 new models)
```

**Plugin List:**
- ExportPlugin (Phase 17) ✅
  - Image Exporter
  - Video Exporter
  - Data Exporter

**Binary Sizes:**
- ExportPlugin.so: **2.0 MB**

### Technical Highlights

#### Image Export Features
- **Multiple Format Support**: PNG, JPEG, BMP, TIFF, WebP
- **Quality Control**: Adjustable compression for lossy formats
- **Auto-Increment**: Sequential filename generation
- **Format-Specific Parameters**:
  - PNG: Lossless (no quality parameter)
  - JPEG: IMWRITE_JPEG_QUALITY
  - BMP: Uncompressed
  - TIFF: LZW compression
  - WebP: IMWRITE_WEBP_QUALITY

#### Video Export Features
- **Codec Selection**: H.264, XVID, MJPG, DIVX
- **Container Formats**: MP4, AVI, MKV, MOV
- **FPS Configuration**: 1-120 fps support
- **Real-Time Recording**: Frame-by-frame capture
- **Status Display**: Frame count during recording

#### Data Export Features
- **Structured Formats**: CSV and JSON
- **Statistical Analysis**: Mean, standard deviation calculation
- **Timestamp Support**: Date/time in filename
- **Preview Window**: Shows data before export
- **Multiple Data Types**: Image info, statistics, histogram

### File Structure

```
plugins/exporters/ExportPlugin/
├── ExportPlugin.h
├── ExportPlugin.cpp
├── ImageExporterModel.h
├── ImageExporterModel.cpp
├── VideoExporterModel.h
├── VideoExporterModel.cpp
├── DataExporterModel.h
├── DataExporterModel.cpp
└── metadata.json
```

### OpenCV Functions Used

**Image Export:**
- `cv::imwrite()` - Save image to file
- `cv::IMWRITE_JPEG_QUALITY` - JPEG quality parameter
- `cv::IMWRITE_WEBP_QUALITY` - WebP quality parameter
- `cv::IMWRITE_TIFF_COMPRESSION` - TIFF compression parameter

**Video Export:**
- `cv::VideoWriter` - Video writer class
- `cv::VideoWriter::fourcc()` - Codec fourcc code
- `cv::VideoWriter::open()` - Open video file for writing
- `cv::VideoWriter::write()` - Write frame to video
- `cv::VideoWriter::isOpened()` - Check if writer is open
- `cv::VideoWriter::release()` - Release video writer

**Data Collection:**
- `cv::meanStdDev()` - Calculate mean and standard deviation
- `cv::minMaxLoc()` - Find minimum and maximum values
- `cv::Mat::channels()` - Get number of channels
- `cv::Mat::depth()` - Get bit depth
- `cv::Mat::size()` - Get image dimensions

### Node Model Count Progress

| Phase | Plugins | Node Models | Total |
|-------|---------|-------------|-------|
| Phase 1-16 | 26 | 50 | 50 |
| Phase 17 | 1 | 3 | **53** |
| **Total** | **27** | **53** | **53** |

### Key Achievements

✅ Image export with 5 format support
✅ Video recording with 6 format/codec combinations
✅ Data export to CSV and JSON
✅ Auto-increment filename feature
✅ Timestamp inclusion in filenames
✅ Quality control for lossy formats
✅ Real-time video recording
✅ Statistical data collection
✅ Preview window for data inspection
✅ 3 new node models in Phase 17
✅ All 27 plugins load successfully
✅ **53 node models total!**

### Use Cases by Model

**ImageExporterModel:**
- Saving filtered/processed images
- Exporting analysis results
- Creating image datasets
- Benchmarking outputs
- Frame extraction from video
- Result documentation
- Batch image saving
- Format conversion

**VideoExporterModel:**
- Recording processed video streams
- Creating demonstration videos
- Saving surveillance footage
- Exporting analysis videos
- Motion capture recording
- Time-lapse creation
- Video format conversion
- Real-time processing output

**DataExporterModel:**
- Statistical analysis logging
- Benchmarking data collection
- Research data export
- Measurement recording
- Quality metrics export
- Integration with data analysis tools
- Machine learning dataset creation
- Experiment documentation

### Dependencies

No new dependencies were added in Phase 17. All features use existing:
- Qt6 (Core, Widgets, Gui)
- OpenCV 4.6.0 (core, imgproc, imgcodecs, videoio)
- QtNodes 3.0.12

**Note:** Phase 17 uses standard Qt and OpenCV functionality:
- `QFileDialog` for file/directory selection
- `QTextStream` for CSV writing
- `QJsonDocument` for JSON writing
- `cv::imwrite()` for image export
- `cv::VideoWriter` for video recording

### Testing

**Build Test:**
```bash
cmake ..
make -j$(nproc)
```
✅ All 27 plugins compile without errors

**Plugin Loading Test:**
```bash
./VisionBox
```
✅ All 27 plugins load successfully

**Plugin Loading Output:**
```
Initialized plugin: "export"
Loaded plugin: "Export Plugin" v "1.0.0" ( "export" )
Total plugins loaded: 27
```

**Binary Sizes:**
- ExportPlugin.so: 2.0 MB

### Completion Date
January 27, 2026

---

**Phase 17 Status: ✅ COMPLETE**

VisionBox now includes comprehensive export functionality for images, videos, and data. Users can save processing results in multiple formats, record video streams, and export statistical data for analysis. These export capabilities are essential for research workflows, result documentation, and integration with other tools.

**🎉 Milestone: 53 node models across 27 plugins!**

**Next Phase Options:**
- Phase 18: Advanced detectors (YOLO, GrabCut segmentation)
- Phase 19: Visualization enhancements (bounding boxes, keypoint viewers)
- Phase 20: Documentation and testing
