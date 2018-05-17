#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QMap>

QT_BEGIN_NAMESPACE
class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    void addPage(QString const& label, QWidget *page);

private:
    QTreeWidget *navTree;
    QMap<QTreeWidgetItem *, QWidget *> navTreeItems;
    QStackedWidget *pageStack;

protected:
    QDialogButtonBox *buttonBox;
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // SETTINGSDIALOG_H
