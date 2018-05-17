#include <QtWidgets>

#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    // Navigation Tree
    navTree = new QTreeWidget();
    navTree->setUniformRowHeights(true);
    navTree->setHeaderHidden(true);
    navTree->setColumnCount(1);

    // Stacked Widget
    pageStack = new QStackedWidget;
	
	// Button Box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close);

    // Connections
    connect(navTree->selectionModel(), &QItemSelectionModel::currentChanged,
        [=](const QModelIndex& current, const QModelIndex& previous) {
        pageStack->setCurrentIndex(current.row());
    });

    installEventFilter(this);

    // Layout
    QSplitter *splitter = new QSplitter;
    splitter->addWidget(navTree);
    splitter->addWidget(pageStack);
    splitter->setOrientation(Qt::Horizontal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void SettingsDialog::addPage(QString const& label, QWidget *page)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, label);
    navTree->addTopLevelItem(item);
    navTreeItems[item] = page;
    pageStack->addWidget(page);
}

bool SettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        // Ignore escape key
        if (ke->key() != Qt::Key_Escape)
            return QObject::eventFilter(obj, event);

        return true;
    }
    else {
        return QObject::eventFilter(obj, event);
    }
    return false;
}
