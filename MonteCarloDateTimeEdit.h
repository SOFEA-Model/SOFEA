#ifndef MONTECARLODATETIMEEDIT_H
#define MONTECARLODATETIMEEDIT_H

#include <QDateTimeEdit>

class MonteCarloDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    explicit MonteCarloDateTimeEdit(QWidget *parent = nullptr);
    bool modified = false;
    bool isModified() const;
    void setModified(bool);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MONTECARLODATETIMEEDIT_H
