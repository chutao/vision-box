/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Main Window Implementation
 ******************************************************************************/

#include "MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QUndoStack>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/NodeDelegateModelRegistry>

#include "DataFlowGraphModel.h"
#include "NodePalette.h"
#include "VisionBoxGraphicsView.h"
#include "PerformancePanel.h"
#include "core/PluginManager.h"

namespace VisionBox
{

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_scene(nullptr), m_view(nullptr), m_graphModel(nullptr),
      m_modified(false)
{
  // Get plugin manager instance
  m_pluginManager = std::shared_ptr<PluginManager>(PluginManager::instance(),
                                                   [](PluginManager*)
                                                   {
                                                     // Don't delete the singleton
                                                   });

  // Add development plugin directory (build directory)
  QString buildDir = qApp->applicationDirPath() + "/plugins";
  m_pluginManager->addPluginDirectory(buildDir);

  // Load plugins from all directories
  for (const QString& pluginDir : m_pluginManager->getPluginDirectories())
  {
    m_pluginManager->loadPluginsFromDirectory(pluginDir);
  }

  // Setup UI components
  setupUi();            // Create scene, view, and graph model
  createMenus();        // Create menu bar
  createToolBar();      // Create tool bar
  createStatusBar();    // Create status bar
  createConnections();  // Connect signals and slots

  // Set window properties
  resize(1280, 800);
  updateWindowTitle();
}

MainWindow::~MainWindow()
{
  // Qt will handle cleanup of child widgets
}

/*******************************************************************************
 * UI Setup
 ******************************************************************************/
void MainWindow::setupUi()
{
  // Create graph model
  m_graphModel = new DataFlowGraphModel(m_pluginManager);

  // Create flow scene (node editor canvas)
  m_scene = new ::QtNodes::DataFlowGraphicsScene(*m_graphModel, this);

  // Create graphics view with context menu disabled
  m_view = new VisionBoxGraphicsView(m_scene, this);

  setCentralWidget(m_view);

  // Create node palette (toolbox)
  auto registry = m_graphModel->registry();
  m_nodePalette = new NodePalette(registry, m_scene, this);
  addDockWidget(Qt::LeftDockWidgetArea, m_nodePalette);

  // Create performance panel
  m_performancePanel = new PerformancePanel(this);

  // Wrap performance panel in a dock widget
  m_performanceDock = new QDockWidget("Performance Statistics", this);
  m_performanceDock->setWidget(m_performancePanel);
  m_performanceDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

  // Sync menu action with dock widget visibility
  connect(m_performanceDock, &QDockWidget::visibilityChanged,
          [this](bool visible)
          {
              m_togglePerformancePanelAction->setChecked(visible);
          });

  addDockWidget(Qt::RightDockWidgetArea, m_performanceDock);
  m_performanceDock->hide();  // Hidden by default
}

void MainWindow::createMenus()
{
  // File Menu
  QMenu* fileMenu = menuBar()->addMenu("&File");

  m_newAction = fileMenu->addAction("&New");
  m_newAction->setShortcut(QKeySequence::New);
  m_newAction->setStatusTip("Create a new node graph");

  m_openAction = fileMenu->addAction("&Open...");
  m_openAction->setShortcut(QKeySequence::Open);
  m_openAction->setStatusTip("Open an existing node graph");

  fileMenu->addSeparator();

  m_saveAction = fileMenu->addAction("&Save");
  m_saveAction->setShortcut(QKeySequence::Save);
  m_saveAction->setStatusTip("Save the current node graph");
  m_saveAction->setEnabled(false);

  m_saveAsAction = fileMenu->addAction("Save &As...");
  m_saveAsAction->setShortcut(QKeySequence::SaveAs);
  m_saveAsAction->setStatusTip("Save the node graph with a new name");

  fileMenu->addSeparator();

  m_exitAction = fileMenu->addAction("E&xit");
  m_exitAction->setShortcut(QKeySequence::Quit);
  m_exitAction->setStatusTip("Exit the application");

  // Edit Menu
  QMenu* editMenu = menuBar()->addMenu("&Edit");

  m_undoAction = editMenu->addAction("&Undo");
  m_undoAction->setShortcut(QKeySequence::Undo);
  m_undoAction->setStatusTip("Undo last action");

  m_redoAction = editMenu->addAction("&Redo");
  m_redoAction->setShortcut(QKeySequence::Redo);
  m_redoAction->setStatusTip("Redo last undone action");

  editMenu->addSeparator();

  m_clearGraphAction = editMenu->addAction("&Clear Graph");
  m_clearGraphAction->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));
  m_clearGraphAction->setStatusTip("Clear all nodes from the graph");

  // View Menu
  QMenu* viewMenu = menuBar()->addMenu("&View");

  m_zoomInAction = viewMenu->addAction("Zoom &In");
  m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
  m_zoomInAction->setStatusTip("Zoom in on the graph");

  m_zoomOutAction = viewMenu->addAction("Zoom &Out");
  m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
  m_zoomOutAction->setStatusTip("Zoom out from the graph");

  m_fitViewAction = viewMenu->addAction("&Fit View");
  m_fitViewAction->setShortcut(QKeySequence(tr("Ctrl+F")));
  m_fitViewAction->setStatusTip("Fit the entire graph in the view");

  viewMenu->addSeparator();

  m_toggleStatusBarAction = viewMenu->addAction("&Status Bar");
  m_toggleStatusBarAction->setCheckable(true);
  m_toggleStatusBarAction->setChecked(true);
  m_toggleStatusBarAction->setStatusTip("Show/hide the status bar");

  m_togglePerformancePanelAction = viewMenu->addAction("&Performance Panel");
  m_togglePerformancePanelAction->setCheckable(true);
  m_togglePerformancePanelAction->setChecked(false);
  m_togglePerformancePanelAction->setShortcut(QKeySequence(tr("Ctrl+P")));
  m_togglePerformancePanelAction->setStatusTip("Show/hide the performance statistics panel");

  // Plugins Menu
  QMenu* pluginsMenu = menuBar()->addMenu("&Plugins");

  m_loadPluginsAction = pluginsMenu->addAction("&Load Plugins...");
  m_loadPluginsAction->setStatusTip("Load plugins from a directory");

  m_pluginInfoAction = pluginsMenu->addAction("&Plugin Info...");
  m_pluginInfoAction->setStatusTip("Show information about loaded plugins");

  // Help Menu
  QMenu* helpMenu = menuBar()->addMenu("&Help");

  m_aboutAction = helpMenu->addAction("&About VisionBox");
  m_aboutAction->setStatusTip("Show information about VisionBox");

  m_aboutQtAction = helpMenu->addAction("About &Qt");
  m_aboutQtAction->setStatusTip("Show information about Qt");
}

void MainWindow::createToolBar()
{
  QToolBar* toolBar = addToolBar("Main");
  toolBar->setMovable(false);

  toolBar->addAction(m_newAction);
  toolBar->addAction(m_openAction);
  toolBar->addAction(m_saveAction);

  toolBar->addSeparator();

  toolBar->addAction(m_undoAction);
  toolBar->addAction(m_redoAction);

  toolBar->addSeparator();

  toolBar->addAction(m_zoomInAction);
  toolBar->addAction(m_zoomOutAction);
  toolBar->addAction(m_fitViewAction);
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage("Ready");
}

void MainWindow::createConnections()
{
  connect(m_newAction, &QAction::triggered, this, &MainWindow::onNew);
  connect(m_openAction, &QAction::triggered, this, &MainWindow::onOpen);
  connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSave);
  connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAs);
  connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);

  connect(m_undoAction, &QAction::triggered,
          [this]()
          {
            if (m_scene)
              m_scene->undoStack().undo();
          });
  connect(m_redoAction, &QAction::triggered,
          [this]()
          {
            if (m_scene)
              m_scene->undoStack().redo();
          });
  connect(m_clearGraphAction, &QAction::triggered, this, &MainWindow::onClearGraph);

  connect(m_zoomInAction, &QAction::triggered, this, &MainWindow::onZoomIn);
  connect(m_zoomOutAction, &QAction::triggered, this, &MainWindow::onZoomOut);
  connect(m_fitViewAction, &QAction::triggered, this, &MainWindow::onFitView);
  connect(m_toggleStatusBarAction, &QAction::triggered, this, &MainWindow::onToggleStatusBar);
  connect(m_togglePerformancePanelAction, &QAction::triggered, this, &MainWindow::onTogglePerformancePanel);

  connect(m_loadPluginsAction, &QAction::triggered, this, &MainWindow::onLoadPlugins);
  connect(m_pluginInfoAction, &QAction::triggered, this, &MainWindow::onPluginInfo);

  connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
  connect(m_aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

  // Connect graph model signals for modification tracking
  if (m_graphModel)
  {
    connect(m_graphModel, &::QtNodes::AbstractGraphModel::nodeCreated, this,
            [this]() { setModified(true); });
    connect(m_graphModel, &::QtNodes::AbstractGraphModel::nodeDeleted, this,
            [this]() { setModified(true); });
    connect(m_graphModel, &::QtNodes::AbstractGraphModel::connectionCreated, this,
            [this]() { setModified(true); });
    connect(m_graphModel, &::QtNodes::AbstractGraphModel::connectionDeleted, this,
            [this]() { setModified(true); });
    connect(m_graphModel, &::QtNodes::AbstractGraphModel::nodePositionUpdated, this,
            [this]() { setModified(true); });
  }
}

/*******************************************************************************
 * Event Handlers
 ******************************************************************************/
void MainWindow::closeEvent(QCloseEvent* event)
{
  if (maybeSave())
  {
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

/*******************************************************************************
 * File Menu Actions
 ******************************************************************************/
void MainWindow::onNew()
{
  if (maybeSave())
  {
    m_scene->clearScene();
    m_currentFile.clear();
    setModified(false);
    m_saveAction->setEnabled(false);
    statusBar()->showMessage("New graph created", 3000);
  }
}

void MainWindow::onOpen()
{
  if (!maybeSave())
  {
    return;
  }

  // Force update of graphics view before showing dialog (fixes Wayland rendering issue)
  if (m_view)
  {
    m_view->update();
    m_view->viewport()->update();
    QApplication::processEvents();
  }

  QFileDialog dialog(this, "Open Node Graph");
  dialog.setNameFilter("VisionBox Graphs (*.vbjson);;All Files (*.*)");
  dialog.setAcceptMode(QFileDialog::AcceptOpen);

  // Set reasonable minimum and initial size to prevent truncation
  dialog.setMinimumSize(800, 600);
  dialog.resize(900, 700);

  if (dialog.exec() != QDialog::Accepted)
  {
    return;
  }

  QString fileName = dialog.selectedFiles().first();
  if (fileName.isEmpty())
  {
    return;
  }

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    QMessageBox::warning(this, "Error", QString("Could not open file: %1").arg(fileName));
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  m_scene->clearScene();
  m_graphModel->load(doc.object());

  m_currentFile = fileName;
  setModified(false);
  m_saveAction->setEnabled(false);
  statusBar()->showMessage(QString("Loaded %1").arg(fileName), 3000);
}

void MainWindow::onSave()
{
  if (m_currentFile.isEmpty())
  {
    onSaveAs();
    return;
  }

  QJsonDocument doc(m_graphModel->save());

  QFile file(m_currentFile);
  if (!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::warning(this, "Error", QString("Could not save file: %1").arg(m_currentFile));
    return;
  }

  file.write(doc.toJson());
  file.close();

  setModified(false);
  m_saveAction->setEnabled(false);
  statusBar()->showMessage(QString("Saved %1").arg(m_currentFile), 3000);
}

void MainWindow::onSaveAs()
{
  // Force update of graphics view before showing dialog (fixes Wayland rendering issue)
  if (m_view)
  {
    m_view->update();
    m_view->viewport()->update();
    QApplication::processEvents();
  }

  QFileDialog dialog(this, "Save Node Graph");
  dialog.setNameFilter("VisionBox Graphs (*.vbjson);;All Files (*.*)");
  dialog.setDefaultSuffix("vbjson");
  dialog.setAcceptMode(QFileDialog::AcceptSave);

  // Set reasonable minimum and initial size to prevent truncation
  dialog.setMinimumSize(800, 600);
  dialog.resize(900, 700);

  if (dialog.exec() != QDialog::Accepted)
  {
    return;
  }

  QString fileName = dialog.selectedFiles().first();
  if (fileName.isEmpty())
  {
    return;
  }

  // Manually ensure .vbjson extension (setDefaultSuffix may not work in all cases)
  if (!fileName.endsWith(".vbjson", Qt::CaseInsensitive))
  {
    fileName += ".vbjson";
  }

  m_currentFile = fileName;
  onSave();
}

void MainWindow::onExit()
{
  if (maybeSave())
  {
    qApp->quit();
  }
}

/*******************************************************************************
 * Edit Menu Actions
 ******************************************************************************/
void MainWindow::onUndo()
{
  m_scene->undoStack().undo();
}

void MainWindow::onRedo()
{
  m_scene->undoStack().redo();
}

void MainWindow::onClearGraph()
{
  if (QMessageBox::question(this, "Clear Graph",
                            "Are you sure you want to clear all nodes from the graph?",
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
  {
    m_scene->clearScene();
    m_modified = true;
    m_saveAction->setEnabled(true);
    updateWindowTitle();
    statusBar()->showMessage("Graph cleared", 3000);
  }
}

/*******************************************************************************
 * View Menu Actions
 ******************************************************************************/
void MainWindow::onZoomIn()
{
  if (m_view)
  {
    m_view->scaleDown();
  }
}

void MainWindow::onZoomOut()
{
  if (m_view)
  {
    m_view->scaleUp();
  }
}

void MainWindow::onFitView()
{
  if (m_view)
  {
    m_view->centerScene();
  }
}

void MainWindow::onToggleStatusBar()
{
  if (statusBar()->isHidden())
  {
    statusBar()->show();
  }
  else
  {
    statusBar()->hide();
  }
}

void MainWindow::onTogglePerformancePanel()
{
  if (m_performanceDock)
  {
    if (m_performanceDock->isHidden())
    {
      m_performanceDock->show();
      m_togglePerformancePanelAction->setChecked(true);
    }
    else
    {
      m_performanceDock->hide();
      m_togglePerformancePanelAction->setChecked(false);
    }
  }
}

/*******************************************************************************
 * Help Menu Actions
 ******************************************************************************/
void MainWindow::onAbout()
{
  QMessageBox::about(this, "About VisionBox",
                     "<h2>VisionBox 1.0.0</h2>"
                     "<p>A modular, plugin-based visual programming framework "
                     "for computer vision research.</p>"
                     "<p>Built with Qt6 and OpenCV.</p>"
                     "<p>Phase 1: Infrastructure</p>");
}

void MainWindow::onAboutQt()
{
  qApp->aboutQt();
}

/*******************************************************************************
 * Plugin Management
 ******************************************************************************/
void MainWindow::onLoadPlugins()
{
  QString dir = QFileDialog::getExistingDirectory(this, "Select Plugin Directory", QString(),
                                                  QFileDialog::ShowDirsOnly |
                                                      QFileDialog::DontResolveSymlinks);

  if (!dir.isEmpty())
  {
    int loaded = m_pluginManager->loadPluginsFromDirectory(dir);
    QMessageBox::information(this, "Plugins Loaded",
                             QString("Loaded %1 plugins from %2").arg(loaded).arg(dir));
    updateStatusBar();
  }
}

void MainWindow::onPluginInfo()
{
  QVector<PluginInfo> plugins = m_pluginManager->getLoadedPlugins();

  QString info;
  info += QString("<h3>Loaded Plugins (%1)</h3>").arg(plugins.size());

  for (const auto& plugin : plugins)
  {
    info += QString("<h4>%1 v%2</h4>").arg(plugin.name).arg(plugin.version);
    info += QString("<p>ID: %1</p>").arg(plugin.id);
    if (!plugin.description.isEmpty())
    {
      info += QString("<p>%1</p>").arg(plugin.description);
    }
    if (!plugin.author.isEmpty())
    {
      info += QString("<p>Author: %1</p>").arg(plugin.author);
    }
    info += QString("<p>Categories: %1</p>").arg(plugin.categories.join(", "));
  }

  if (plugins.isEmpty())
  {
    info += "<p>No plugins loaded.</p>";
  }

  QMessageBox::about(this, "Plugin Information", info);
}

/*******************************************************************************
 * Helper Methods
 ******************************************************************************/
void MainWindow::updateWindowTitle(const QString& fileName)
{
  QString title = "VisionBox";

  if (!fileName.isEmpty())
  {
    title += " - " + QFileInfo(fileName).fileName();
  }
  else if (m_modified)
  {
    title += " - Untitled";
  }

  if (m_modified)
  {
    title += " *";
  }

  setWindowTitle(title);
}

void MainWindow::setModified(bool modified)
{
  if (m_modified != modified)
  {
    m_modified = modified;
    updateWindowTitle();
  }
}

void MainWindow::updateStatusBar()
{
  QVector<PluginInfo> plugins = m_pluginManager->getLoadedPlugins();
  QString message = QString("Ready | Plugins: %1").arg(plugins.size());

  if (!plugins.isEmpty())
  {
    QStringList categories = m_pluginManager->getAllCategories();
    message += QString(" | Categories: %1").arg(categories.size());
  }

  statusBar()->showMessage(message);
}

bool MainWindow::maybeSave()
{
  if (!m_modified)
  {
    return true;
  }

  QMessageBox::StandardButton ret = QMessageBox::warning(
      this, "Save Changes?",
      "The current graph has been modified.\nDo you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  switch (ret)
  {
  case QMessageBox::Save:
    onSave();
    return !m_modified;

  case QMessageBox::Discard:
    return true;

  case QMessageBox::Cancel:
    return false;

  default:
    return false;
  }
}

}  // namespace VisionBox
