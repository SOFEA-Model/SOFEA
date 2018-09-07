#ifndef MONTECARLODATETIMEEDIT_H
#define MONTECARLODATETIMEEDIT_H

#include <QAction>
#include <QDateTimeEdit>
#include <QEvent>
#include <QKeyEvent>

#include "DateTimeDistribution.h"

class MonteCarloDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT

public:
    explicit MonteCarloDateTimeEdit(QWidget *parent = nullptr);

    bool isModified() const;
    void setModified(bool);
    DateTimeDistribution getDistribution() const;
    void setDistribution(const DateTimeDistribution &d);

private:
    QAction *selectDistributionAct;
    bool modified = false;
    DateTimeDistribution currentDist;
    QDateTime previousValue;
    bool firstShowEvent = true;

    void reset();
    void clearDistribution();
    void resetToolButtonPosition();

private slots:
    void onSelectDistribution();
    void onCursorPositionChanged(int oldPos, int newPos);
    void onTextEdited(const QString &);
    void onDateTimeChanged(const QDateTime &dt);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MONTECARLODATETIMEEDIT_H
