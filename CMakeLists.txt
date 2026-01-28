cmake_minimum_required(VERSION 3.16)

################################################################################
# VisionBox - Computer Vision Research Framework
# Phase 1: Infrastructure
################################################################################

project(VisionBox
    VERSION 1.0.0
    DESCRIPTION "A modular, plugin-based visual programming framework for computer vision research"
    LANGUAGES CXX
)

################################################################################
# C++ Standard
################################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

################################################################################
# Build Options
################################################################################
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_PLUGINS "Build built-in plugins" ON)
option(ENABLE_CLANG_TOOLS "Enable clang-format and clang-tidy targets" ON)

################################################################################
# Installation Directories
################################################################################
include(GNUInstallDirs)
set(INSTALL_BIN_DIR     ${CMAKE_INSTALL_BINDIR})
set(INSTALL_LIB_DIR     ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(INSTALL_CMAKE_DIR   ${CMAKE_INSTALL_LIBDIR}/cmake/VisionBox)

################################################################################
# Find Dependencies - Qt
################################################################################
# Try Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Found Qt6: ${Qt6_VERSION}")
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets)
    # Enable automatic MOC, UIC and RCC processing
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
else()
    find_package(Qt5 5.12 REQUIRED COMPONENTS Core Widgets)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Found Qt5: ${Qt5_VERSION}")
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets)
    # Enable automatic MOC, UIC and RCC processing for Qt5
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

################################################################################
# Find Dependencies - OpenCV
################################################################################
find_package(OpenCV 4.0 REQUIRED COMPONENTS
    core
    imgproc
    imgcodecs
    videoio
    objdetect
    bgsegm
)
message(STATUS "Found OpenCV: ${OpenCV_VERSION}")
message(STATUS "  OpenCV libraries: ${OpenCV_LIBS}")

################################################################################
# Add QtNodes as External Project
################################################################################
set(QT_NODES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/QtNodes")
if(EXISTS "${QT_NODES_PATH}/CMakeLists.txt")
    message(STATUS "Using QtNodes from ${QT_NODES_PATH}")

    # Build QtNodes as a subdirectory
    set(USE_QT6 ON CACHE BOOL "Use Qt6" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
    set(BUILD_DOCS OFF CACHE BOOL "Build docs" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)

    add_subdirectory(${QT_NODES_PATH} ${CMAKE_BINARY_DIR}/QtNodes)

    # Create alias for consistency (QtNodes already creates it)
    # if(TARGET QtNodes)
    #     add_library(QtNodes::QtNodes ALIAS QtNodes)
    # endif()

else()
    message(WARNING "QtNodes not found at ${QT_NODES_PATH}")
    message(WARNING "Please clone: git clone https://github.com/QtNodes/QtNodes.git external/QtNodes")
endif()

################################################################################
# Generate compile_commands.json for language servers
################################################################################
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

################################################################################
# Clang Tools Integration
################################################################################
if(ENABLE_CLANG_TOOLS)
    # Find clang-format
    find_program(CLANG_FORMAT_BIN clang-format)
    if(CLANG_FORMAT_BIN)
        message(STATUS "Found clang-format: ${CLANG_FORMAT_BIN}")

        # Create format target
        add_custom_target(format
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/format_code.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting code with clang-format"
        )

        # Create check-format target
        add_custom_target(check-format
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/check_format.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Checking code formatting with clang-format"
        )
    else()
        message(WARNING "clang-format not found - code formatting targets disabled")
    endif()

    # Find clang-tidy
    find_program(CLANG_TIDY_BIN clang-tidy)
    if(CLANG_TIDY_BIN)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_BIN}")

        # Create tidy target
        add_custom_target(tidy
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/run_tidy.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Running static analysis with clang-tidy"
        )
    else()
        message(WARNING "clang-tidy not found - static analysis target disabled")
    endif()
endif()

################################################################################
# Source Files
################################################################################
set(VISIONBOX_CORE_SOURCES
    src/core/PluginManager.cpp
    src/core/VisionDataTypes.cpp
    src/core/PerformanceMonitor.cpp
)

set(VISIONBOX_CORE_HEADERS
    src/core/PluginInterface.h
    src/core/PluginManager.h
    src/core/VisionDataTypes.h
    src/core/NodeError.h
    src/core/PerformanceMonitor.h
)

set(VISIONBOX_UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/DataFlowGraphModel.cpp
    src/ui/NodePalette.cpp
    src/ui/NodePaletteTreeWidget.cpp
    src/ui/VisionBoxGraphicsView.cpp
    src/ui/PerformancePanel.cpp
)

set(VISIONBOX_UI_HEADERS
    src/ui/MainWindow.h
    src/ui/DataFlowGraphModel.h
    src/ui/NodePalette.h
    src/ui/NodePaletteTreeWidget.h
    src/ui/VisionBoxGraphicsView.h
    src/ui/PerformancePanel.h
)

set(VISIONBOX_MAIN
    src/main.cpp
)

################################################################################
# VisionBox Executable Target
################################################################################
add_executable(VisionBox
    ${VISIONBOX_MAIN}
    ${VISIONBOX_CORE_SOURCES}
    ${VISIONBOX_CORE_HEADERS}
    ${VISIONBOX_UI_SOURCES}
    ${VISIONBOX_UI_HEADERS}
)

# Set target properties
set_target_properties(VisionBox PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME "VisionBox"
    CXX_STANDARD 20
)

# Include directories
target_include_directories(VisionBox PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/QtNodes/include
    ${CMAKE_SOURCE_DIR}/external/QtNodes/src
    ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(VisionBox PRIVATE
    ${QT_LIBRARIES}
    QtNodes::QtNodes
    ${OpenCV_LIBS}
)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(VisionBox PRIVATE dl)
    # Export symbols for plugin loading
    set_target_properties(VisionBox PROPERTIES
        LINK_FLAGS "-Wl,--export-dynamic"
    )
endif()

# Enable clang-tidy for the target (if available)
if(CLANG_TIDY_BIN)
    set_target_properties(VisionBox PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_BIN};-format-style=file;-p=${CMAKE_BINARY_DIR}"
    )
endif()

################################################################################
# Testing
################################################################################
if(BUILD_TESTS)
    enable_testing()
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test QUIET)
    if(Qt${QT_VERSION_MAJOR}Test_FOUND)
        message(STATUS "Qt Test found - building unit tests")

        add_executable(DataTypesTest
            tests/unit/DataTypesTest.cpp
            ${VISIONBOX_CORE_SOURCES}
            ${VISIONBOX_CORE_HEADERS}
        )

        target_include_directories(DataTypesTest PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/external/QtNodes/include
            ${CMAKE_SOURCE_DIR}/external/QtNodes/src
            ${OpenCV_INCLUDE_DIRS}
        )

        target_link_libraries(DataTypesTest PRIVATE
            ${QT_LIBRARIES}
            Qt${QT_VERSION_MAJOR}::Test
            QtNodes::QtNodes
            ${OpenCV_LIBS}
        )

        add_test(NAME DataTypesTest COMMAND DataTypesTest)
    else()
        message(WARNING "Qt Test not found - unit tests disabled")
    endif()
endif()

################################################################################
# Plugins - Phase 2
################################################################################
if(BUILD_PLUGINS)
    message(STATUS "Building plugins")

    # ImageSourcePlugin (Phase 2 + Phase 13)
    add_library(ImageSourcePlugin SHARED
        plugins/sources/ImageSourcePlugin/ImageSourcePlugin.cpp
        plugins/sources/ImageSourcePlugin/ImageLoaderModel.cpp
        plugins/sources/ImageSourcePlugin/VideoLoaderModel.cpp
        plugins/sources/ImageSourcePlugin/CameraSourceModel.cpp
        plugins/sources/ImageSourcePlugin/ImageGeneratorModel.cpp
    )

    target_include_directories(ImageSourcePlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/sources/ImageSourcePlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ImageSourcePlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ImageSourcePlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""  # Remove lib prefix on plugins
    )

    # Install plugin
    install(TARGETS ImageSourcePlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # Install plugin metadata
    install(FILES
        plugins/sources/ImageSourcePlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # BasicFilterPlugin
    add_library(BasicFilterPlugin SHARED
        plugins/filters/BasicFilterPlugin/BasicFilterPlugin.cpp
        plugins/filters/BasicFilterPlugin/BlurModel.cpp
        plugins/filters/BasicFilterPlugin/ThresholdModel.cpp
        plugins/filters/BasicFilterPlugin/MorphologyModel.cpp
        plugins/filters/BasicFilterPlugin/ColorConvertModel.cpp
    )

    target_include_directories(BasicFilterPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/filters/BasicFilterPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(BasicFilterPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(BasicFilterPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS BasicFilterPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/filters/BasicFilterPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # ImageViewerPlugin
    add_library(ImageViewerPlugin SHARED
        plugins/displays/ImageViewerPlugin/ImageViewerPlugin.cpp
        plugins/displays/ImageViewerPlugin/ImageViewerModel.cpp
    )

    target_include_directories(ImageViewerPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/displays/ImageViewerPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ImageViewerPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ImageViewerPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ImageViewerPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/displays/ImageViewerPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # EdgeDetectionPlugin (Phase 3)
    add_library(EdgeDetectionPlugin SHARED
        plugins/features/EdgeDetectionPlugin/EdgeDetectionPlugin.cpp
        plugins/features/EdgeDetectionPlugin/CannyModel.cpp
        plugins/features/EdgeDetectionPlugin/SobelModel.cpp
        plugins/features/EdgeDetectionPlugin/LaplacianModel.cpp
        plugins/features/EdgeDetectionPlugin/ScharrModel.cpp
    )

    target_include_directories(EdgeDetectionPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/features/EdgeDetectionPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(EdgeDetectionPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(EdgeDetectionPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS EdgeDetectionPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/features/EdgeDetectionPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # GeometricTransformPlugin (Phase 3)
    add_library(GeometricTransformPlugin SHARED
        plugins/transforms/GeometricTransformPlugin/GeometricTransformPlugin.cpp
        plugins/transforms/GeometricTransformPlugin/ResizeModel.cpp
        plugins/transforms/GeometricTransformPlugin/RotateModel.cpp
        plugins/transforms/GeometricTransformPlugin/FlipModel.cpp
    )

    target_include_directories(GeometricTransformPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/transforms/GeometricTransformPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(GeometricTransformPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(GeometricTransformPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS GeometricTransformPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/transforms/GeometricTransformPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    ################################################################################
    # AdvancedTransformPlugin (Phase 15)
    ################################################################################
    option(BUILD_ADVANCEDTRANSFORM_PLUGIN "Build AdvancedTransformPlugin" ON)
    if(BUILD_ADVANCEDTRANSFORM_PLUGIN)
        add_library(AdvancedTransformPlugin SHARED
            plugins/transforms/AdvancedTransformPlugin/AdvancedTransformPlugin.cpp
            plugins/transforms/AdvancedTransformPlugin/AffineTransformModel.cpp
            plugins/transforms/AdvancedTransformPlugin/PerspectiveTransformModel.cpp
        )

        target_include_directories(AdvancedTransformPlugin PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/plugins/transforms/AdvancedTransformPlugin
            ${CMAKE_SOURCE_DIR}/external/QtNodes/include
            ${CMAKE_SOURCE_DIR}/external/QtNodes/src
            ${OpenCV_INCLUDE_DIRS}
        )

        target_link_libraries(AdvancedTransformPlugin PRIVATE
            ${QT_LIBRARIES}
            QtNodes::QtNodes
            ${OpenCV_LIBS}
        )

        set_target_properties(AdvancedTransformPlugin PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
            PREFIX ""
        )

        install(TARGETS AdvancedTransformPlugin
            LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
            RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        )

        install(FILES
            plugins/transforms/AdvancedTransformPlugin/metadata.json
            DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        )
    endif()

    # ImageArithmeticPlugin (Phase 4)
    add_library(ImageArithmeticPlugin SHARED
        plugins/arithmetic/ImageArithmeticPlugin/ImageArithmeticPlugin.cpp
        plugins/arithmetic/ImageArithmeticPlugin/BinaryOpModel.cpp
    )

    target_include_directories(ImageArithmeticPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/arithmetic/ImageArithmeticPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ImageArithmeticPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ImageArithmeticPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ImageArithmeticPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/arithmetic/ImageArithmeticPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # ColorAdjustmentPlugin (Phase 4)
    add_library(ColorAdjustmentPlugin SHARED
        plugins/color/ColorAdjustmentPlugin/ColorAdjustmentPlugin.cpp
        plugins/color/ColorAdjustmentPlugin/BrightnessContrastModel.cpp
        plugins/color/ColorAdjustmentPlugin/SaturationModel.cpp
    )

    target_include_directories(ColorAdjustmentPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/color/ColorAdjustmentPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ColorAdjustmentPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ColorAdjustmentPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ColorAdjustmentPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/color/ColorAdjustmentPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # HistogramPlugin (Phase 5)
    add_library(HistogramPlugin SHARED
        plugins/histogram/HistogramPlugin/HistogramPlugin.cpp
        plugins/histogram/HistogramPlugin/HistogramEqualizationModel.cpp
    )

    target_include_directories(HistogramPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/histogram/HistogramPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(HistogramPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(HistogramPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS HistogramPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/histogram/HistogramPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # EnhancementPlugin (Phase 5)
    add_library(EnhancementPlugin SHARED
        plugins/enhancement/EnhancementPlugin/EnhancementPlugin.cpp
        plugins/enhancement/EnhancementPlugin/DenoiseModel.cpp
        plugins/enhancement/EnhancementPlugin/SharpenModel.cpp
    )

    target_include_directories(EnhancementPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/enhancement/EnhancementPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(EnhancementPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(EnhancementPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS EnhancementPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/enhancement/EnhancementPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # SegmentationPlugin (Phase 6)
    add_library(SegmentationPlugin SHARED
        plugins/segmentation/SegmentationPlugin/SegmentationPlugin.cpp
        plugins/segmentation/SegmentationPlugin/KMeansSegmentationModel.cpp
    )

    target_include_directories(SegmentationPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/segmentation/SegmentationPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(SegmentationPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(SegmentationPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS SegmentationPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/segmentation/SegmentationPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # ContourPlugin (Phase 6)
    add_library(ContourPlugin SHARED
        contours/ContourPlugin/ContourPlugin.cpp
        contours/ContourPlugin/ContourFinderModel.cpp
    )

    target_include_directories(ContourPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/contours/ContourPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ContourPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ContourPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ContourPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        contours/ContourPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # FeatureDetectionPlugin (Phase 7)
    add_library(FeatureDetectionPlugin SHARED
        plugins/features/FeatureDetectionPlugin/FeatureDetectionPlugin.cpp
        plugins/features/FeatureDetectionPlugin/CornerDetectionModel.cpp
    )

    target_include_directories(FeatureDetectionPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/features/FeatureDetectionPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(FeatureDetectionPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(FeatureDetectionPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS FeatureDetectionPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/features/FeatureDetectionPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    # TemplateMatchingPlugin (Phase 7)
    add_library(TemplateMatchingPlugin SHARED
        plugins/matching/TemplateMatchingPlugin/TemplateMatchingPlugin.cpp
        plugins/matching/TemplateMatchingPlugin/TemplateMatchingModel.cpp
    )

    target_include_directories(TemplateMatchingPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/matching/TemplateMatchingPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(TemplateMatchingPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(TemplateMatchingPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS TemplateMatchingPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/matching/TemplateMatchingPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# OpticalFlowPlugin (Phase 8)
################################################################################
option(BUILD_OPTICALFLOW_PLUGIN "Build OpticalFlowPlugin" ON)
if(BUILD_OPTICALFLOW_PLUGIN)
    add_library(OpticalFlowPlugin SHARED
        plugins/opticalflow/OpticalFlowPlugin/OpticalFlowPlugin.cpp
        plugins/opticalflow/OpticalFlowPlugin/OpticalFlowModel.cpp
    )

    target_include_directories(OpticalFlowPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/opticalflow/OpticalFlowPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(OpticalFlowPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(OpticalFlowPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS OpticalFlowPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/opticalflow/OpticalFlowPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# FeatureDescriptorsPlugin (Phase 8)
################################################################################
option(BUILD_FEATUREDESCRIPTORS_PLUGIN "Build FeatureDescriptorsPlugin" ON)
if(BUILD_FEATUREDESCRIPTORS_PLUGIN)
    add_library(FeatureDescriptorsPlugin SHARED
        plugins/features/FeatureDescriptorsPlugin/FeatureDescriptorsPlugin.cpp
        plugins/features/FeatureDescriptorsPlugin/ORBFeatureModel.cpp
    )

    target_include_directories(FeatureDescriptorsPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/features/FeatureDescriptorsPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(FeatureDescriptorsPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(FeatureDescriptorsPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS FeatureDescriptorsPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/features/FeatureDescriptorsPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# FeatureMatchingPlugin (Phase 14)
################################################################################
option(BUILD_FEATUREMATCHING_PLUGIN "Build FeatureMatchingPlugin" ON)
if(BUILD_FEATUREMATCHING_PLUGIN)
    add_library(FeatureMatchingPlugin SHARED
        plugins/features/FeatureMatchingPlugin/FeatureMatchingPlugin.cpp
        plugins/features/FeatureMatchingPlugin/SIFTFeaturesModel.cpp
        plugins/features/FeatureMatchingPlugin/AKAZEFeaturesModel.cpp
        plugins/features/FeatureMatchingPlugin/FeatureMatcherModel.cpp
    )

    target_include_directories(FeatureMatchingPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/features/FeatureMatchingPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(FeatureMatchingPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(FeatureMatchingPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS FeatureMatchingPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/features/FeatureMatchingPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# VideoProcessingPlugin (Phase 9)
################################################################################
option(BUILD_VIDEOPROCESSING_PLUGIN "Build VideoProcessingPlugin" ON)
if(BUILD_VIDEOPROCESSING_PLUGIN)
    add_library(VideoProcessingPlugin SHARED
        plugins/video/VideoProcessingPlugin/VideoProcessingPlugin.cpp
        plugins/video/VideoProcessingPlugin/BackgroundSubtractionModel.cpp
    )

    target_include_directories(VideoProcessingPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/video/VideoProcessingPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(VideoProcessingPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(VideoProcessingPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS VideoProcessingPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/video/VideoProcessingPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# ObjectDetectionPlugin (Phase 9)
################################################################################
option(BUILD_OBJECTDETECTION_PLUGIN "Build ObjectDetectionPlugin" ON)
if(BUILD_OBJECTDETECTION_PLUGIN)
    add_library(ObjectDetectionPlugin SHARED
        plugins/detection/ObjectDetectionPlugin/ObjectDetectionPlugin.cpp
        plugins/detection/ObjectDetectionPlugin/HOGDetectionModel.cpp
    )

    target_include_directories(ObjectDetectionPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/detection/ObjectDetectionPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ObjectDetectionPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ObjectDetectionPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ObjectDetectionPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/detection/ObjectDetectionPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# ImagePyramidPlugin (Phase 10)
################################################################################
option(BUILD_IMAGEPYRAMID_PLUGIN "Build ImagePyramidPlugin" ON)
if(BUILD_IMAGEPYRAMID_PLUGIN)
    add_library(ImagePyramidPlugin SHARED
        plugins/pyramids/ImagePyramidPlugin/ImagePyramidPlugin.cpp
        plugins/pyramids/ImagePyramidPlugin/ImagePyramidModel.cpp
    )

    target_include_directories(ImagePyramidPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/pyramids/ImagePyramidPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ImagePyramidPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ImagePyramidPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ImagePyramidPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/pyramids/ImagePyramidPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# FaceDetectionPlugin (Phase 10)
################################################################################
option(BUILD_FACEDETECTION_PLUGIN "Build FaceDetectionPlugin" ON)
if(BUILD_FACEDETECTION_PLUGIN)
    add_library(FaceDetectionPlugin SHARED
        plugins/face/FaceDetectionPlugin/FaceDetectionPlugin.cpp
        plugins/face/FaceDetectionPlugin/HaarFaceDetectionModel.cpp
    )

    target_include_directories(FaceDetectionPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/face/FaceDetectionPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(FaceDetectionPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(FaceDetectionPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS FaceDetectionPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/face/FaceDetectionPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# MachineLearningPlugin (Phase 16)
################################################################################
option(BUILD_MACHINELEARNING_PLUGIN "Build MachineLearningPlugin" ON)
if(BUILD_MACHINELEARNING_PLUGIN)
    add_library(MachineLearningPlugin SHARED
        plugins/ml/MachineLearningPlugin/MachineLearningPlugin.cpp
        plugins/ml/MachineLearningPlugin/DNNInferenceModel.cpp
        plugins/ml/MachineLearningPlugin/ObjectTrackerModel.cpp
    )

    target_include_directories(MachineLearningPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/ml/MachineLearningPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(MachineLearningPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(MachineLearningPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS MachineLearningPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/ml/MachineLearningPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# ExportPlugin (Phase 17)
################################################################################
option(BUILD_EXPORT_PLUGIN "Build ExportPlugin" ON)
if(BUILD_EXPORT_PLUGIN)
    add_library(ExportPlugin SHARED
        plugins/exporters/ExportPlugin/ExportPlugin.cpp
        plugins/exporters/ExportPlugin/ImageExporterModel.cpp
        plugins/exporters/ExportPlugin/VideoExporterModel.cpp
        plugins/exporters/ExportPlugin/DataExporterModel.cpp
    )

    target_include_directories(ExportPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/exporters/ExportPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ExportPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ExportPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ExportPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/exporters/ExportPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# AdvancedDetectionPlugin (Phase 18)
################################################################################
option(BUILD_ADVANCEDDETECTION_PLUGIN "Build AdvancedDetectionPlugin" ON)
if(BUILD_ADVANCEDDETECTION_PLUGIN)
    add_library(AdvancedDetectionPlugin SHARED
        plugins/detection/AdvancedDetectionPlugin/AdvancedDetectionPlugin.cpp
        plugins/detection/AdvancedDetectionPlugin/YOLOObjectDetectorModel.cpp
        plugins/detection/AdvancedDetectionPlugin/GrabCutSegmentationModel.cpp
    )

    target_include_directories(AdvancedDetectionPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/detection/AdvancedDetectionPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(AdvancedDetectionPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(AdvancedDetectionPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS AdvancedDetectionPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/detection/AdvancedDetectionPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# VisualizationPlugin (Phase 19)
################################################################################
option(BUILD_VISUALIZATION_PLUGIN "Build VisualizationPlugin" ON)
if(BUILD_VISUALIZATION_PLUGIN)
    add_library(VisualizationPlugin SHARED
        plugins/visualization/VisualizationPlugin/VisualizationPlugin.cpp
        plugins/visualization/VisualizationPlugin/BoundingBoxOverlayModel.cpp
        plugins/visualization/VisualizationPlugin/KeypointViewerModel.cpp
        plugins/visualization/VisualizationPlugin/DrawingOverlayModel.cpp
    )

    target_include_directories(VisualizationPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/visualization/VisualizationPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(VisualizationPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(VisualizationPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS VisualizationPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/visualization/VisualizationPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# AdvancedMorphologyPlugin (Phase 11)
################################################################################
option(BUILD_ADVANCEDMORPHOLOGY_PLUGIN "Build AdvancedMorphologyPlugin" ON)
if(BUILD_ADVANCEDMORPHOLOGY_PLUGIN)
    add_library(AdvancedMorphologyPlugin SHARED
        plugins/morphology/AdvancedMorphologyPlugin/AdvancedMorphologyPlugin.cpp
        plugins/morphology/AdvancedMorphologyPlugin/AdvancedMorphologyModel.cpp
    )

    target_include_directories(AdvancedMorphologyPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/morphology/AdvancedMorphologyPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(AdvancedMorphologyPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(AdvancedMorphologyPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS AdvancedMorphologyPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/morphology/AdvancedMorphologyPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# ColorSpacePlugin (Phase 11)
################################################################################
option(BUILD_COLORSPACE_PLUGIN "Build ColorSpacePlugin" ON)
if(BUILD_COLORSPACE_PLUGIN)
    add_library(ColorSpacePlugin SHARED
        plugins/color/ColorSpacePlugin/ColorSpacePlugin.cpp
        plugins/color/ColorSpacePlugin/ColorSpaceModel.cpp
    )

    target_include_directories(ColorSpacePlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/color/ColorSpacePlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(ColorSpacePlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ColorSpacePlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS ColorSpacePlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/color/ColorSpacePlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# DistanceTransformPlugin (Phase 12)
################################################################################
option(BUILD_DISTANCETRANSFORM_PLUGIN "Build DistanceTransformPlugin" ON)
if(BUILD_DISTANCETRANSFORM_PLUGIN)
    add_library(DistanceTransformPlugin SHARED
        plugins/segmentation/DistanceTransformPlugin/DistanceTransformPlugin.cpp
        plugins/segmentation/DistanceTransformPlugin/DistanceTransformModel.cpp
    )

    target_include_directories(DistanceTransformPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/segmentation/DistanceTransformPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(DistanceTransformPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(DistanceTransformPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS DistanceTransformPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/segmentation/DistanceTransformPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# WatershedSegmentationPlugin (Phase 12)
################################################################################
option(BUILD_WATERSHEDSEGMENTATION_PLUGIN "Build WatershedSegmentationPlugin" ON)
if(BUILD_WATERSHEDSEGMENTATION_PLUGIN)
    add_library(WatershedSegmentationPlugin SHARED
        plugins/segmentation/WatershedSegmentationPlugin/WatershedSegmentationPlugin.cpp
        plugins/segmentation/WatershedSegmentationPlugin/WatershedSegmentationModel.cpp
    )

    target_include_directories(WatershedSegmentationPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/plugins/segmentation/WatershedSegmentationPlugin
        ${CMAKE_SOURCE_DIR}/external/QtNodes/include
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
        ${OpenCV_INCLUDE_DIRS}
    )

    target_link_libraries(WatershedSegmentationPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(WatershedSegmentationPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
        PREFIX ""
    )

    install(TARGETS WatershedSegmentationPlugin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
        RUNTIME DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )

    install(FILES
        plugins/segmentation/WatershedSegmentationPlugin/metadata.json
        DESTINATION ${INSTALL_LIB_DIR}/vision-box/plugins
    )
endif()

################################################################################
# Installation
################################################################################
install(TARGETS VisionBox
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
)

# Install launcher script for Wayland compatibility
install(FILES
    VisionBox.sh
    DESTINATION ${INSTALL_BIN_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install(FILES
    ${VISIONBOX_CORE_HEADERS}
    ${VISIONBOX_UI_HEADERS}
    DESTINATION ${INSTALL_INCLUDE_DIR}/VisionBox
)

# Install CMake config
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/VisionBoxConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/VisionBoxConfig.cmake
    @ONLY
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/VisionBoxConfig.cmake
    DESTINATION ${INSTALL_CMAKE_DIR}
)

################################################################################
# Summary
################################################################################
message(STATUS "")
message(STATUS "VisionBox Configuration Summary")
message(STATUS "===============================")
message(STATUS "  Version:            ${PROJECT_VERSION}")
message(STATUS "  Qt version:         Qt${QT_VERSION_MAJOR}")
message(STATUS "  OpenCV version:     ${OpenCV_VERSION}")
message(STATUS "  Build tests:        ${BUILD_TESTS}")
message(STATUS "  Enable clang tools: ${ENABLE_CLANG_TOOLS}")
message(STATUS "  Install prefix:     ${CMAKE_INSTALL_PREFIX}")
message(STATUS "===============================")
message(STATUS "")
