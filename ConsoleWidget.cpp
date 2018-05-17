#include "ConsoleWidget.h"

#include <QMenu>

ConsoleWidget::ConsoleWidget(QWidget *parent) : PlainTextEdit(parent)
{
    // Context Menu
    clearAct = new QAction(tr("Clear"), this);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

void ConsoleWidget::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    QMenu *contextMenu = this->createStandardContextMenu();

    contextMenu->addSeparator();
    contextMenu->addAction(clearAct);

    QAction *selectedItem = contextMenu->exec(globalPos);
    if (selectedItem && selectedItem == clearAct)
        this->clear();

    contextMenu->deleteLater();
}
