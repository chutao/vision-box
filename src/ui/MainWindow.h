/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Main Window
 ******************************************************************************/

#ifndef VISIONBOX_MAINWINDOW_H
#define VISIONBOX_MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QDockWidget>
#include <memory>

// Forward declarations
namespace VisionBox {
class NodePalette;
class VisionBoxGraphicsView;
class PerformancePanel;
}

// Forward declarations for QtNodes classes (must be outside VisionBox namespace)
namespace QtNodes {
class DataFlowGraphicsScene;
}

namespace VisionBox {

class PluginManager;
class DataFlowGraphModel;
class NodePalette;

/*******************************************************************************
 * MainWindow - Main application window
 ******************************************************************************/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // File menu actions
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onExit();

    // Edit menu actions
    void onUndo();
    void onRedo();
    void onClearGraph();

    // View menu actions
    void onZoomIn();
    void onZoomOut();
    void onFitView();
    void onToggleStatusBar();
    void onTogglePerformancePanel();

    // Help menu actions
    void onAbout();
    void onAboutQt();

    // Plugin management
    void onLoadPlugins();
    void onPluginInfo();

private:
    void setupUi();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createConnections();

    void updateWindowTitle(const QString& fileName = QString());
    void updateStatusBar();
    void setModified(bool modified);
    bool maybeSave();

    // UI Components
    QtNodes::DataFlowGraphicsScene* m_scene;
    VisionBoxGraphicsView* m_view;
    DataFlowGraphModel* m_graphModel;
    NodePalette* m_nodePalette;
    PerformancePanel* m_performancePanel;
    QDockWidget* m_performanceDock;
    std::shared_ptr<PluginManager> m_pluginManager;

    // Actions
    QAction* m_newAction;
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_exitAction;

    QAction* m_undoAction;
    QAction* m_redoAction;
    QAction* m_clearGraphAction;

    QAction* m_zoomInAction;
    QAction* m_zoomOutAction;
    QAction* m_fitViewAction;
    QAction* m_toggleStatusBarAction;
    QAction* m_togglePerformancePanelAction;

    QAction* m_loadPluginsAction;
    QAction* m_pluginInfoAction;

    QAction* m_aboutAction;
    QAction* m_aboutQtAction;

    // State
    QString m_currentFile;
    bool m_modified;
};

} // namespace VisionBox

#endif // VISIONBOX_MAINWINDOW_H
