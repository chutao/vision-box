/*******************************************************************************
 * VisionBox - Computer Vision Research Framework
 * Node Palette Implementation
 ******************************************************************************/

#include "NodePalette.h"
#include "NodePaletteTreeWidget.h"
#include <QtNodes/NodeDelegateModelRegistry>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QtGui/QDrag>
#include <QtCore/QMimeData>
#include <QtCore/QDebug>
#include <QGraphicsView>
#include <QPointF>

namespace VisionBox {

/*******************************************************************************
 * Constructor / Destructor
 ******************************************************************************/
NodePalette::NodePalette(std::shared_ptr<::QtNodes::NodeDelegateModelRegistry> registry,
                         ::QtNodes::DataFlowGraphicsScene* scene,
                         QWidget* parent)
    : QDockWidget("Node Palette", parent)
    , m_scene(scene)
    , m_registry(registry)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    setupUi();
    populateTree();
}

NodePalette::~NodePalette()
{
}

/*******************************************************************************
 * UI Setup
 ******************************************************************************/
void NodePalette::setupUi()
{
    // Create main widget
    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    // Create filter edit
    QLabel* filterLabel = new QLabel("Filter:", container);
    layout->addWidget(filterLabel);

    m_filterEdit = new QLineEdit(container);
    m_filterEdit->setPlaceholderText("Type to filter nodes...");
    m_filterEdit->setClearButtonEnabled(true);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &NodePalette::onFilterChanged);
    layout->addWidget(m_filterEdit);

    // Create tree widget with drag support
    m_treeWidget = new NodePaletteTreeWidget(container);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setDragEnabled(true);  // Enable drag from palette
    m_treeWidget->setDragDropMode(QAbstractItemView::DragOnly);
    m_treeWidget->setAcceptDrops(false);  // Don't accept drops in palette
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked,
            this, &NodePalette::onItemDoubleClicked);
    layout->addWidget(m_treeWidget);

    // Set as dock widget content
    setWidget(container);
}

/*******************************************************************************
 * Population
 ******************************************************************************/
void NodePalette::populateTree()
{
    if (!m_registry) {
        return;
    }

    m_treeWidget->clear();
    m_categoryNodes.clear();

    // Get all categories and model-category associations
    auto categories = m_registry->categories();
    auto modelCategories = m_registry->registeredModelsCategoryAssociation();

    // Group models by category
    for (auto const &assoc : modelCategories)
    {
        QString modelName = assoc.first;  // Model name
        QString category = assoc.second;   // Category name

        NodeInfo info;
        info.name = modelName;
        info.caption = modelName;  // Use model name as caption

        m_categoryNodes[category].append(info);
    }

    // Populate tree
    for (QString const &category : categories)
    {
        if (!m_categoryNodes.contains(category)) {
            continue;
        }

        // Create category item
        auto categoryItem = new QTreeWidgetItem(m_treeWidget);
        categoryItem->setText(0, category);
        categoryItem->setFlags(categoryItem->flags() & ~Qt::ItemIsSelectable);
        categoryItem->setExpanded(true);

        // Add nodes in this category
        for (NodeInfo const &node : m_categoryNodes[category])
        {
            auto nodeItem = new QTreeWidgetItem(categoryItem);
            nodeItem->setText(0, node.caption);
            nodeItem->setData(0, Qt::UserRole, node.name);
            nodeItem->setIcon(0, QIcon::fromTheme("code-block"));  // Generic icon
        }
    }

    // Expand all categories
    m_treeWidget->expandAll();
}

/*******************************************************************************
 * Actions
 ******************************************************************************/
void NodePalette::refresh()
{
    populateTree();
}

void NodePalette::onFilterChanged(const QString& text)
{
    QString filter = text.toLower();

    // Hide/show items based on filter
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* categoryItem = m_treeWidget->topLevelItem(i);
        bool categoryHasVisibleChildren = false;

        for (int j = 0; j < categoryItem->childCount(); ++j)
        {
            QTreeWidgetItem* nodeItem = categoryItem->child(j);
            QString nodeName = nodeItem->text(0).toLower();

            bool matches = filter.isEmpty() || nodeName.contains(filter);
            nodeItem->setHidden(!matches);

            if (matches) {
                categoryHasVisibleChildren = true;
            }
        }

        // Hide category if no visible children
        categoryItem->setHidden(!categoryHasVisibleChildren);
    }
}

void NodePalette::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);

    // Only process leaf items (nodes, not categories)
    if (!item->parent()) {
        return;
    }

    QString modelName = item->data(0, Qt::UserRole).toString();
    if (modelName.isEmpty()) {
        return;
    }

    if (m_scene) {
        // Create node using the graph model
        qDebug() << "NodePalette: Creating node" << modelName;

        // Get the graph model from the scene
        auto& graphModel = m_scene->graphModel();

        // Add node to the graph - the registry will create the actual model instance
        ::QtNodes::NodeId nodeId = graphModel.addNode(modelName);

        // NodeId is an unsigned int, so 0 is invalid
        if (nodeId == 0) {
            qWarning() << "Failed to create node:" << modelName;
            return;
        }

        qDebug() << "NodePalette: Successfully created node" << modelName << "with ID" << nodeId;
    }
}

} // namespace VisionBox
