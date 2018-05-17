#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include "Utilities.h"

#include <QAction>

class ConsoleWidget : public PlainTextEdit
{
    Q_OBJECT
    
public:
    explicit ConsoleWidget(QWidget *parent = nullptr);
    
public slots:
    void showContextMenu(const QPoint &pos);
    
private:
    QAction *clearAct;
};

#endif // CONSOLEWIDGET_H
