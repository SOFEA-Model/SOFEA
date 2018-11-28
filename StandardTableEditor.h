#ifndef STANDARDTABLEEDITOR_H
#define STANDARDTABLEEDITOR_H

#include <QBoxLayout>
#include <QFlags>
#include <QPushButton>
#include "StandardTableView.h"

class StandardTableEditor : public QWidget
{
    Q_OBJECT

public:
    enum StandardButton {
        Add       = 0x1,
        Remove    = 0x2,
        AddRemove = 0x3,
        Rename    = 0x4,
        MoveUp    = 0x8,
        MoveDown  = 0x10,
        Edit      = 0x20,
        Import    = 0x40,
        All       = 0xff
    };

    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    StandardTableEditor(Qt::Orientation orientation = Qt::Vertical,
        StandardButtons buttons = AddRemove, QWidget *parent = nullptr);

    void init(StandardTableView *standardTableView);
    void setImportFilter(const QString& filter);
    void setImportCaption(const QString& caption);
    QString importFile();

    QPushButton *btnAdd = nullptr;
    QPushButton *btnRemove = nullptr;
    QPushButton *btnRename = nullptr;
    QPushButton *btnMoveUp = nullptr;
    QPushButton *btnMoveDown = nullptr;
    QPushButton *btnEdit = nullptr;
    QPushButton *btnImport = nullptr;

signals:
    void moveRequested(const QModelIndex &sourceParent, int sourceRow, int count,
                       const QModelIndex &destinationParent, int destinationRow);
    void editRequested(const QModelIndex &index);
    void importRequested();

public slots:
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onRenameItemClicked();;
    void onMoveRequested();
    void onEditItemClicked();
    void onImportClicked();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    StandardTableView *m_standardTableView;
    QString m_importFilter;
    QString m_importCaption;
    QString m_importFile;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StandardTableEditor::StandardButtons)

#endif // STANDARDTABLEEDITOR_H
