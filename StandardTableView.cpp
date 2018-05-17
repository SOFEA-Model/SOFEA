#include "StandardTableView.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QModelIndex>

StandardTableView::StandardTableView(QWidget *parent) : QTableView(parent)
{
    QHeaderView *hHeader = horizontalHeader();
    QHeaderView *vHeader = verticalHeader();

    QFont headerFont = hHeader->font();
    headerFont.setPointSize(8);

    hHeader->setFont(headerFont);
    hHeader->setFixedHeight(28);
    hHeader->setStretchLastSection(true);

    vHeader->setFont(headerFont);
    vHeader->setSectionResizeMode(QHeaderView::Fixed);
    vHeader->setDefaultSectionSize(24);
    vHeader->setVisible(false);

    setSelectionBehavior(QAbstractItemView::SelectRows);
}

void StandardTableView::addRow()
{
    if (!model())
        return;

    int nrows = model()->rowCount();
    model()->insertRow(nrows);
}

void StandardTableView::selectLastRow()
{
    if (!model())
        return;

    int nrows = model()->rowCount();
    if (nrows > 0)
        selectRow(nrows - 1);
}

void StandardTableView::removeSelectedRows()
{
    if (!model() || !selectionModel())
        return;

    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();

    std::sort(selectedIndexes.begin(), selectedIndexes.end(),
        [](const QModelIndex& a, const QModelIndex& b)->bool {
        return b.row() < a.row();
    });

    QSet<int> deletedRows;
    for (auto i = selectedIndexes.constBegin(); i != selectedIndexes.constEnd(); ++i)
    {
        if (deletedRows.contains(i->row()))
            continue; // row already deleted
        if (model()->removeRow(i->row(), i->parent())) // try to delete the row
            deletedRows << i->row(); // add to deleted set
    }
}

void StandardTableView::copyClipboard()
{
    QString clipboardString;
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();

    QMap<int, bool> columnMap;
    for (QModelIndex& current : selectedIndexes)
        columnMap[current.column()] = true;
    QList<int> columns = columnMap.uniqueKeys();

    for (int column : columns) {
        clipboardString += model()->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
        if (column != columns.last())
            clipboardString += QLatin1Char('\t');
    }

    clipboardString += QLatin1Char('\n');

    for (int i = 0; i < selectedIndexes.size(); ++i)
    {
        QModelIndex currentIndex = selectedIndexes[i];
        QString displayText = currentIndex.data(Qt::DisplayRole).toString();

        // Check if this is second to last column.
        if (i + 1 < selectedIndexes.size())
        {
            QModelIndex nextIndex = selectedIndexes[i+1];
            // Append newline if the new column is on a new row.
            if (nextIndex.row() != currentIndex.row())
                displayText.append(QLatin1Char('\n'));
            else
                displayText.append(QLatin1Char('\t'));
        }

        clipboardString.append(displayText);
    }

    QApplication::clipboard()->setText(clipboardString);
}

void StandardTableView::setDoubleLineEdit(double min, double max, int decimals, bool fixed)
{
    setItemDelegate(new DoubleItemDelegate(min, max, decimals, fixed));
}

void StandardTableView::setDoubleLineEditForColumn(int column, double min, double max, int decimals, bool fixed)
{
    setItemDelegateForColumn(column, new DoubleItemDelegate(min, max, decimals, fixed));
}

void StandardTableView::setSpinBox(int min, int max)
{
    setItemDelegate(new SpinBoxDelegate(min, max));
}

void StandardTableView::setSpinBoxForColumn(int column, int min, int max)
{
    setItemDelegateForColumn(column, new SpinBoxDelegate(min, max));
}

void StandardTableView::setDoubleSpinBox(double min, double max, int decimals, bool fixed)
{
    setItemDelegate(new DoubleSpinBoxDelegate(min, max, decimals, fixed));
}

void StandardTableView::setDoubleSpinBoxForColumn(int column, double min, double max, int decimals, bool fixed)
{
    setItemDelegateForColumn(column, new DoubleSpinBoxDelegate(min, max, decimals, fixed));
}

void StandardTableView::setDateTimeEdit(QDateTime min, QDateTime max)
{
    setItemDelegate(new DateTimeEditDelegate(min, max));
}

void StandardTableView::setDateTimeEditForColumn(int column, QDateTime min, QDateTime max)
{
    setItemDelegateForColumn(column, new DateTimeEditDelegate(min, max));
}

std::vector<double> StandardTableView::getNumericColumn(int j)
{
    std::vector<double> values;

    if (!model())
        return values;

    int nrows = model()->rowCount();
    int ncols = model()->columnCount();

    if (j < 0 || j >= ncols)
        return values;

    for (int i = 0; i < nrows; ++i) {
        QModelIndex currentIndex = model()->index(i, j);
        QVariant data = model()->data(currentIndex);
        bool ok;
        double value = data.toDouble(&ok);
        if (ok)
            values.push_back(value);
    }

    return values;
}

template <typename T>
void StandardTableView::setColumnData(int column, const std::vector<T> &values)
{
    if (!model())
        return;

    int nrows = model()->rowCount();
    int ncols = model()->columnCount();

    if (column < 0 || column >= ncols)
        return;

    if (values.size() != nrows)
        return;

    for (int row = 0; row < nrows; ++row) {
        QModelIndex currentIndex = model()->index(row, column);
        model()->setData(currentIndex, values[row], Qt::DisplayRole);
    }

    return;
}

// Template Specialization
template void StandardTableView::setColumnData<int>(int, const std::vector<int> &);
template void StandardTableView::setColumnData<double>(int, const std::vector<double> &);

void StandardTableView::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy)) {
        copyClipboard();
        event->accept();
    }
    else {
        QTableView::keyPressEvent(event);
    }
}
