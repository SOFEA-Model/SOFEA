#ifndef INPUTVIEWER_H
#define INPUTVIEWER_H

#include "Scenario.h"
#include "Utilities.h"

#include <QAction>

class InputViewer : public PlainTextEdit
{
    Q_OBJECT

public:
    explicit InputViewer(Scenario *s);

public slots:
    void refresh();

private slots:
    void showContextMenu(const QPoint &pos);

private:
    Scenario *sPtr;
    QAction *refreshAct;
};

#endif // INPUTVIEWER_H
