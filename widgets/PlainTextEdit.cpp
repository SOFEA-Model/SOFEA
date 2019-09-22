#include "PlainTextEdit.h"

#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QMargins>
#include <QString>
#include <QStylePainter>
#include <QTextDocument>
#include <QTextOption>
#include <QWidget>

PlainTextEdit::PlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QFont font;
    font.setFamily("Consolas");
    font.setStyleHint(QFont::TypeWriter);
    font.setFixedPitch(true);
    font.setPointSize(QApplication::font().pointSize());
    setFont(font);
}

void PlainTextEdit::setLineCount(int lines)
{
    const QTextDocument *doc = this->document();
    const QFontMetrics fm = this->fontMetrics();
    const QMargins margins = this->contentsMargins();

    int height = fm.lineSpacing() * lines +
        (doc->documentMargin() + this->frameWidth()) * 2 +
        margins.top() + margins.bottom();

    this->setFixedHeight(height);
}

void PlainTextEdit::appendPlainText(const std::string &text)
{
    QString qstr = QString::fromStdString(text);
    QPlainTextEdit::appendPlainText(qstr);
}

void PlainTextEdit::insertPlainText(const std::string &text)
{
    QString qstr = QString::fromStdString(text);
    QPlainTextEdit::insertPlainText(qstr);
}

void PlainTextEdit::setPlainText(const std::string &text)
{
    QString qstr = QString::fromStdString(text);
    QPlainTextEdit::setPlainText(qstr);
}
