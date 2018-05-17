#include "InputViewer.h"

#include <string>

#include <QMenu>
#include <QScrollBar>

InputViewer::InputViewer(Scenario *s) : sPtr(s)
{
    // Context Menu
    const QIcon refreshIcon = QIcon(":/images/Refresh_16x.png");
    refreshAct = new QAction(refreshIcon, tr("Refresh"), this);
    connect(refreshAct, &QAction::triggered, this, &InputViewer::refresh);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

void InputViewer::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    QMenu *contextMenu = this->createStandardContextMenu();

    contextMenu->addSeparator();
    contextMenu->addAction(refreshAct);

    QAction *selectedItem = contextMenu->exec(globalPos);
    if (selectedItem && selectedItem == refreshAct)
        this->refresh();

    contextMenu->deleteLater();
}

void InputViewer::refresh()
{
    if (sPtr == nullptr)
        return;

    // Capture the current scroll state
    QScrollBar *vsb = verticalScrollBar();
    int vsbval = vsb->value();

    std::string buffer = sPtr->writeInput();
    setPlainText(buffer);

    // Restore scroll state
    vsb->setValue(vsbval);
}
