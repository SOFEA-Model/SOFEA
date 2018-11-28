#ifndef SOURCEMODEL_H
#define SOURCEMODEL_H

#include <QAbstractTableModel>
#include <QVariant>

#include "SourceGroup.h"

class SourceModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SourceModel(SourceGroup *sg, QObject *parent = nullptr);
    void reset();
    void import();
    Source* sourceFromIndex(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &index = QModelIndex()) override;

public slots:
    void addAreaSource();
    void addAreaCircSource();
    void addAreaPolySource();

private:
    SourceGroup *sgPtr;
};

#endif // SOURCEMODEL_H
