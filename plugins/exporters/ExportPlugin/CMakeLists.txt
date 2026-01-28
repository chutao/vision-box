# ExportPlugin CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

# ExportPlugin (Phase 17)
option(BUILD_EXPORT_PLUGIN "Build ExportPlugin" ON)

if(BUILD_EXPORT_PLUGIN)
    add_library(ExportPlugin SHARED
        ExportPlugin.cpp
        ExportPlugin.h
        ImageExporterModel.cpp
        ImageExporterModel.h
        VideoExporterModel.cpp
        VideoExporterModel.h
        DataExporterModel.cpp
        DataExporterModel.h
        metadata.json
    )

    target_include_directories(ExportPlugin PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/external/QtNodes/src
    )

    target_link_libraries(ExportPlugin PRIVATE
        ${QT_LIBRARIES}
        QtNodes::QtNodes
        ${OpenCV_LIBS}
    )

    set_target_properties(ExportPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/exporters
        PREFIX ""
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )

    # Export symbols for plugin
    target_compile_definitions(ExportPlugin PRIVATE
        VISIONBOX_EXPORT_LIBRARY
    )
endif()
