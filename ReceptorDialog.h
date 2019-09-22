#ifndef RECEPTORDIALOG_H
#define RECEPTORDIALOG_H

#include <QDialog>
#include <QWidget>

class ReceptorEditor;
struct Scenario;

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QKeyEvent;
class QShowEvent;
QT_END_NAMESPACE

class ReceptorDialog : public QDialog
{
    Q_OBJECT

public:
    ReceptorDialog(Scenario *s, QWidget *parent = nullptr);
    ~ReceptorDialog();

private slots:
    void accept() override;
    void reject() override;

private:
    Scenario *sPtr;
    ReceptorEditor *editor;
    QDialogButtonBox *buttonBox;

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // RECEPTORDIALOG_H
