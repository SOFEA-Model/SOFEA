#ifndef MONTECARLOLINEEDIT_H
#define MONTECARLOLINEEDIT_H

#include "GenericDistribution.h"

#include <QLineEdit>
#include <QDoubleValidator>

class MonteCarloLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MonteCarloLineEdit(QWidget *parent = nullptr);
    void setMinimum(double min);
    void setMaximum(double max);
    void setDecimals(int decimals);
    void setRange(double min, double max);
    GenericDistribution getDistribution() const;
    void setDistribution(const GenericDistribution &d);

private:
    QAction *m_selectDistributionAct;
    GenericDistribution m_distribution;
    QDoubleValidator *m_validator;
    bool m_distributionSet;
    QString m_distributionText;
    double m_previousValue;

    void init();
    void clearDistribution();
    QString getDistributionText() const;

private slots:
    void onSelectDistribution();
    void onCursorPositionChanged(int oldPos, int newPos);
    void onTextEdited(const QString &text);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // MONTECARLOLINEEDIT_H
