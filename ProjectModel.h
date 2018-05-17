#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H
/*
#include <QAbstractItemModel>
#include <QList>
#include <QVariant>

#include "Scenario.h"

class ProjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ProjectModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &index = QModelIndex()) override;

private:

};
*/
#endif // PROJECTMODEL_H
