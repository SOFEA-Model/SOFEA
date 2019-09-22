#include <QApplication>
#include <QIcon>
#include <QLineEdit>
#include <QToolButton>
#include <QDebug>

#include "MonteCarloDateTimeEdit.h"
#include "DateTimeDistributionDialog.h"

MonteCarloDateTimeEdit::MonteCarloDateTimeEdit(QWidget *parent)
    : QDateTimeEdit(parent)
{
    setButtonSymbols(QAbstractSpinBox::NoButtons);

    // Reuse the special value text to indicate a distribution is set.
    setSpecialValueText("Custom"); // minimumDateTime()

    // Actions
    static const QIcon functionIcon = QIcon(":/images/Effects_32x.png");
    selectDistributionAct = new QAction(functionIcon, "Define Distribution");
    lineEdit()->addAction(selectDistributionAct, QLineEdit::TrailingPosition);

    // Connections
    connect(selectDistributionAct, &QAction::triggered, this, &MonteCarloDateTimeEdit::onSelectDistribution);
    connect(lineEdit(), &QLineEdit::textEdited, this, &MonteCarloDateTimeEdit::onTextEdited);
    connect(lineEdit(), &QLineEdit::cursorPositionChanged, this, &MonteCarloDateTimeEdit::onCursorPositionChanged);
    connect(this, &MonteCarloDateTimeEdit::dateTimeChanged, this, &MonteCarloDateTimeEdit::onDateTimeChanged);

    // Defaults
    previousValue = QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC);
    Distribution::ConstantDateTime d;
    d.setTimeSpec(Qt::UTC);
    d.setDate(previousValue.date());
    d.setTime(previousValue.time());
    currentDist = d;
}

bool MonteCarloDateTimeEdit::isModified() const
{
    return modified;
}

void MonteCarloDateTimeEdit::setModified(bool m)
{
    modified = m;
}

void MonteCarloDateTimeEdit::reset()
{
    if (currentDist.which() == 0) {
        auto d = boost::get<Distribution::ConstantDateTime>(currentDist);
        setDateTime(QDateTime(d.date(), d.time(), Qt::UTC));
        QPalette palette = QApplication::palette(this);
        setPalette(palette);
        setSelectedSection(QDateTimeEdit::NoSection);
    }
    else {
        previousValue = dateTime();
        setDateTime(minimumDateTime());
        QPalette palette = this->palette();
        palette.setColor(QPalette::Text, Qt::blue);
        setPalette(palette);
    }
}

DateTimeDistribution MonteCarloDateTimeEdit::getDistribution() const
{
    return currentDist;
}

void MonteCarloDateTimeEdit::setDistribution(const DateTimeDistribution &d)
{    
    currentDist = d;
    reset();
}

void MonteCarloDateTimeEdit::clearDistribution()
{
    Distribution::ConstantDateTime d;
    d.setTimeSpec(Qt::UTC);
    d.setDate(previousValue.date());
    d.setTime(previousValue.time());
    currentDist = d;
    setDateTime(QDateTime(d.date(), d.time(), d.timeSpec()));
    reset();
}

void MonteCarloDateTimeEdit::onSelectDistribution()
{
    DateTimeDistributionDialog dialog(currentDist, this);

    if (QDialog::Accepted == dialog.exec())
    {
        DateTimeDistribution d = dialog.getDistribution();

        // Check whether the distribution actually changed.
        if (d == currentDist)
            setModified(false);
        else
            setModified(true);

        setDistribution(d);
    }
}

void MonteCarloDateTimeEdit::onCursorPositionChanged(int oldPos, int newPos)
{
    // If the distribution is set, make sure the cursor position can
    // only be at the start or end of the text.

    if (currentDist.which() > 0) {
        if (newPos >= oldPos)
            lineEdit()->setCursorPosition(lineEdit()->text().length());
        else
            lineEdit()->setCursorPosition(0);
    }
}

void MonteCarloDateTimeEdit::onTextEdited(const QString &)
{
    // If the distribution is set, ignore edits.

    if (currentDist.which() > 0) {
        setDateTime(minimumDateTime());
    }
}

void MonteCarloDateTimeEdit::onDateTimeChanged(const QDateTime &dt)
{
    // If the distribution is set, ignore edits. Otherwise, set a
    // constant distribution using the current value.

    if (currentDist.which() > 0) {
        setDateTime(minimumDateTime());
    }
    else {
        if (previousValue == dt)
            setModified(false);
        else
            setModified(true);

        previousValue = dt;
        Distribution::ConstantDateTime d;
        d.setTimeSpec(Qt::UTC);
        d.setDate(dt.date());
        d.setTime(dt.time());
        currentDist = d;
    }
}

void MonteCarloDateTimeEdit::resetToolButtonPosition()
{
    // HACK: adjust the tool button position by the x shift
    // of the internal QLineEdit. This must be done after
    // positionSideWidgets() in QLineEditPrivate.

    QLineEdit *le = lineEdit();
    QToolButton *actBtn = nullptr;
    actBtn = le->findChild<QToolButton *>();
    if (actBtn) {
        int x = actBtn->pos().x() + le->pos().x();
        int y = actBtn->pos().y() + 1;
        actBtn->move(x, y);
    }
}

void MonteCarloDateTimeEdit::focusInEvent(QFocusEvent *event)
{
    // If the distribution is set, select all text. We also ensure the
    // cursor can only be at the start or end.

    if (currentDist.which() > 0) {
        selectAll();
    }

    QDateTimeEdit::focusInEvent(event);
}

void MonteCarloDateTimeEdit::keyPressEvent(QKeyEvent *event)
{
    if (currentDist.which() > 0) {
        if ((event->key() == Qt::Key_Delete && lineEdit()->cursorPosition() == 0) ||
            (event->key() == Qt::Key_Backspace && lineEdit()->cursorPosition() > 0)) {
            clearDistribution();
            setModified(true);
            return;
        }
    }

    QDateTimeEdit::keyPressEvent(event);
}

void MonteCarloDateTimeEdit::showEvent(QShowEvent *event)
{
    QDateTimeEdit::showEvent(event);

    if (firstShowEvent) {
        resetToolButtonPosition();
        firstShowEvent = false;
    }
}

void MonteCarloDateTimeEdit::resizeEvent(QResizeEvent *event)
{
    QDateTimeEdit::resizeEvent(event);
    resetToolButtonPosition();
}
