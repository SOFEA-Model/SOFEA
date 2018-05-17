#ifndef STANDARDTABLEEDITOR_H
#define STANDARDTABLEEDITOR_H

#include <QBoxLayout>
#include <QPushButton>
#include "StandardTableView.h"

class StandardTableEditor : public QWidget
{
    Q_OBJECT
public:
    StandardTableEditor(const QBoxLayout::Direction direction, QWidget *parent = nullptr);
    void init(StandardTableView *standardTableView);
    void setControlsEnabled(bool);
    void setImportEnabled(bool);
    void setImportFilter(const QString& filter);
    void setImportCaption(const QString& caption);
    QString importFile();
    void connectActions();
    void disconnectActions();

    QPushButton *btnAdd;
    QPushButton *btnRemove;
    QPushButton *btnImport;

signals:
    void importRequested();

public slots:
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onImportClicked();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    StandardTableView *m_standardTableView;
    QString m_importFilter;
    QString m_importCaption;
    QString m_importFile;
};

#endif // STANDARDTABLEEDITOR_H
