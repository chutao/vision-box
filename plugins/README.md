# VisionBox Plugins

This directory contains built-in and user-created plugins for VisionBox.

## Plugin Development (Phase 2)

Plugin development will be available in Phase 2. This section provides a preview of the plugin architecture.

### Plugin Structure

Each plugin is a shared library (.so, .dll, .dylib) that implements the `IVisionNodePlugin` interface.

### Directory Layout (Phase 2)

```
plugins/
├── source/              # Source node plugins
│   ├── ImageLoader/
│   ├── VideoLoader/
│   └── CameraSource/
├── filter/              # Filter node plugins
│   ├── Blur/
│   ├── Threshold/
│   ├── Morphology/
│   └── ColorConvert/
├── feature/             # Feature detection plugins
│   ├── ORBDetector/
│   └── SIFTDetector/
└── display/             # Display node plugins
    ├── ImageViewer/
    └── Histogram/
```

### Plugin Interface

All plugins must implement the `IVisionNodePlugin` interface defined in `src/core/PluginInterface.h`:

```cpp
class IVisionNodePlugin
{
public:
    virtual QString pluginId() const = 0;
    virtual QString pluginName() const = 0;
    virtual QString pluginVersion() const = 0;
    virtual QStringList categories() const = 0;
    virtual QVector<std::unique_ptr<QtNodes::NodeDelegateModel>> createNodeModels() const = 0;
    virtual void initialize() = 0;
    virtual void cleanup() = 0;
};
```

### Plugin Installation

#### System-wide
```bash
cp libMyPlugin.so /usr/local/lib/VisionBox/plugins/
```

#### User-local
```bash
cp libMyPlugin.so ~/.local/share/VisionBox/plugins/
```

#### Custom directory
```bash
./VisionBox --plugin-dir /path/to/my/plugins
```

## Current Status

Phase 1 provides the plugin infrastructure but no built-in plugins yet. Built-in plugins will be implemented in Phase 2.

### Available in Phase 2:
- ImageLoader - Load images from disk
- VideoLoader - Load video files
- CameraSource - Capture from webcam
- Blur - Gaussian/median blur filters
- Threshold - Binary/adaptive thresholding
- Morphology - Erode/dilate/open/close operations
- ColorConvert - Convert between color spaces
- ImageViewer - Display images in a window
- Histogram - Display image histograms

## Plugin Development Guide

Coming in Phase 2: Complete plugin development tutorial with examples.
