#ifndef UDUNITSLINEEDIT_H
#define UDUNITSLINEEDIT_H

#include <QLineEdit>
#include <QPaintEvent>
#include <QPalette>

class UDUnitsLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit UDUnitsLineEdit(QWidget *parent = nullptr);
    void setBasePalette();
    void setConvertFrom(const QString &unit);
    QString parsedText();
    double scaleFactor();

signals:
    void unitsChanged();

private slots:
    void onTextChanged(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPalette m_defaultPalette;
    QPalette m_errorPalette;
    QString m_parsedText;
    QString m_convertFromText;
    double m_scaleFactor;
};

#endif // UDUNITSLINEEDIT_H
