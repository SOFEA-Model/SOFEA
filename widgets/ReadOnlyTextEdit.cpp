#include "ReadOnlyTextEdit.h"

#include <QColor>
#include <QFontMetrics>
#include <QMargins>
#include <QPalette>
#include <QTextDocument>
#include <QWidget>

ReadOnlyTextEdit::ReadOnlyTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAutoFillBackground(true);

    QPalette palette = this->palette();
    QColor baseColor = QWidget::palette().window().color().lighter(104);
    palette.setColor(QPalette::Base, baseColor);
    this->setPalette(palette);
}

void ReadOnlyTextEdit::setLineCount(int lines)
{
    const QTextDocument *doc = this->document();
    const QFontMetrics fm(doc->defaultFont());
    const QMargins margins = this->contentsMargins();

    int height = fm.lineSpacing() * lines +
        (doc->documentMargin() + this->frameWidth()) * 2 +
        margins.top() + margins.bottom();

    this->setFixedHeight(height);
}
