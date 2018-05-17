#include "MonteCarloDateTimeEdit.h"

#include <QAction>
#include <QEvent>
#include <QIcon>
#include <QLineEdit>

MonteCarloDateTimeEdit::MonteCarloDateTimeEdit(QWidget *parent)
    : QDateTimeEdit(parent)
{
    setTimeSpec(Qt::UTC);

    const QIcon chartIcon = QIcon(":/images/Effects_16x.png");
    QLineEdit *lePtr = this->lineEdit();
    QAction *act = new QAction(chartIcon, "Define Distribution");
    lePtr->addAction(act, QLineEdit::TrailingPosition);

    setButtonSymbols(QAbstractSpinBox::NoButtons);

    installEventFilter(this);
}

bool MonteCarloDateTimeEdit::isModified() const
{
    return modified;
}

void MonteCarloDateTimeEdit::setModified(bool m)
{
    modified = m;
}

bool MonteCarloDateTimeEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::Wheel) {
        setModified(true);
    }
    else {
        return QObject::eventFilter(obj, event);
    }
    return false;
}
