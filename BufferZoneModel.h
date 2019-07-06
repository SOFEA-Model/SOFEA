#ifndef BUFFERZONEMODEL_H
#define BUFFERZONEMODEL_H

#include <set>

#include <QAbstractTableModel>
#include <QVariant>

#include "BufferZone.h"

class BufferZoneModel : public QAbstractTableModel
{
public:
    explicit BufferZoneModel(QObject *parent = nullptr);

    void save(std::set<BufferZone>& zones) const;
    void load(const std::set<BufferZone>& zones);
    void insert(const BufferZone& zone);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &index = QModelIndex()) override;

private:
    std::set<BufferZone> m_zones;
};

#endif // BUFFERZONEMODEL_H
