# VisionBox

A modular, plugin-based visual programming framework for computer vision research using OpenCV and Qt NodeEditor.

## Overview

VisionBox provides a node-based visual programming environment for designing and executing computer vision pipelines. Its plugin-based architecture allows for easy extension with custom nodes and algorithms.

## Features

- **Visual Node-Based Programming**: Design CV pipelines using a drag-and-drop interface
- **Plugin Architecture**: Extensible system for adding custom nodes
- **OpenCV Integration**: Built-in support for OpenCV operations
- **Qt 6 Support**: Native Qt 6.4 with full Wayland support
- **Modern C++**: C++20 standard with smart pointers and RAII
- **29 Built-in Plugins**: 56 node models covering all major CV operations

## Phase 1: Infrastructure

This implementation represents Phase 1 of VisionBox, establishing the core foundation:

### Implemented Components

- [x] Project structure and CMake build system
- [x] Clang toolchain integration (clang-format, clang-tidy)
- [x] Qt NodeEditor integration
- [x] Core data types (ImageData, DetectionData, KeypointData)
- [x] Plugin interface and manager
- [x] Basic main window UI with node editor canvas
- [x] Unit tests for data types

### Planned for Phase 2

- Source node plugins (ImageLoader, VideoLoader, CameraSource)
- Basic filter plugins (Blur, Threshold, Morphology, ColorConvert)
- Display node plugins (ImageViewer, Histogram)
- Pipeline serialization (save/load node graphs)

## System Requirements

### Dependencies

- **CMake**: 3.16 or higher
- **C++ Compiler**: C++20 compliant (GCC 10+, Clang 12+, MSVC 2019+)
- **Qt**: 6.0+ (preferred) or 5.12+ (Core, Widgets, Test modules)
- **OpenCV**: 4.0+ (core, imgproc, imgcodecs, videoio modules)

**Qt 6 Recommendation**: Qt 6.4+ provides native Wayland support and improved performance.

### Optional Dependencies

- **clang-format**: For code formatting
- **clang-tidy**: For static analysis

## Building

### 1. Clone Repository and Initialize Submodule

```bash
git clone <repository-url>
cd vision-box
git submodule update --init --recursive
```

**Note:** If the QtNodes submodule was not properly initialized, you may need to add it manually:

```bash
git submodule add https://github.com/QtNodes/QtNodes.git external/QtNodes
git submodule update --init --recursive
```

### 2. Create Build Directory

```bash
mkdir build && cd build
```

### 3. Configure with CMake

```bash
cmake ..
```

**Options:**
- `-DBUILD_TESTS=OFF` to disable unit tests
- `-DENABLE_CLANG_TOOLS=OFF` to disable clang-format/clang-tidy targets

### 4. Build

```bash
cmake --build .
```

### 5. Run Tests (Optional)

```bash
ctest --output-on-failure
```

### 6. Run Application

```bash
./VisionBox
```

**Qt 6 + Wayland**: If you have Qt 6.4+, the application will use native Wayland backend with full dialog support. No additional configuration needed.

**Qt 5.15**: For Qt 5.15 on Wayland, see [docs/WAYLAND_COMPATIBILITY.md](docs/WAYLAND_COMPATIBILITY.md) for compatibility options.

## Usage

### Command Line Options

```bash
# Load plugins from custom directory
./VisionBox --plugin-dir /path/to/plugins

# List all loaded plugins
./VisionBox --list-plugins

# Disable automatic plugin loading
./VisionBox --no-auto-load
```

### Basic Workflow

1. **Load Plugins**: Plugins are automatically loaded from default directories
   - System: `<install-dir>/plugins`
   - User: `~/.local/share/VisionBox/plugins` (Linux)
   - Or use Plugins → Load Plugins... menu

2. **Add Nodes**: Right-click on the canvas to add nodes from loaded plugins

3. **Connect Nodes**: Drag between node ports to establish data flow

4. **Save/Load**: Use File menu to save and load node graphs

## Development

### Code Style

VisionBox uses clang-format for consistent code style:

```bash
# Format all code
./scripts/format_code.sh

# Check formatting
./scripts/check_format.sh

# Run static analysis
./scripts/run_tidy.sh
```

### Git Hooks

Install pre-commit and pre-push hooks:

```bash
./scripts/setup_git_hooks.sh
```

### Plugin Development

See `plugins/README.md` for plugin development guide (available in Phase 2).

## Project Structure

```
vision-box/
├── CMakeLists.txt          # Root build configuration
├── .clang-format           # Code formatting config
├── .clang-tidy             # Static analysis config
├── scripts/                # Utility scripts
├── src/                    # Core application
│   ├── core/              # Core functionality
│   │   ├── PluginInterface.h    # Plugin interface
│   │   ├── PluginManager.{h,cpp} # Plugin manager
│   │   └── VisionDataTypes.{h,cpp} # Data types
│   ├── ui/                # User interface
│   │   ├── MainWindow.{h,cpp}
│   │   └── DataFlowGraphModel.h
│   └── main.cpp
├── plugins/               # Built-in plugins (Phase 2)
├── external/              # External dependencies
│   └── QtNodes/           # Qt NodeEditor submodule
├── tests/                 # Unit tests
└── cmake/                 # CMake modules
```

## License

[Specify your license here]

## Contributing

[Specify contribution guidelines here]

## Roadmap

- **Phase 1**: Infrastructure (Current)
- **Phase 2**: Core Plugins
- **Phase 3**: Advanced Features
- **Phase 4**: Optimization
- **Phase 5**: Multi-threading and GPU support

## Acknowledgments

- [Qt Nodes](https://github.com/QtNodes/QtNodes) - Node Editor library
- [OpenCV](https://opencv.org/) - Computer vision library
- [Qt](https://www.qt.io/) - Application framework
