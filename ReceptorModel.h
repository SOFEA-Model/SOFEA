#ifndef RECEPTORMODEL_H
#define RECEPTORMODEL_H

#include <memory>
#include <vector>

#include <QAbstractItemModel>
#include <QVariant>

#include "Receptor.h"

struct SourceGroup;

class ReceptorModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        Color,
        Group,
        Type,
        X,
        Y,
        Z,
        ZHill,
        ZFlag
    };

    ReceptorModel(QObject *parent = nullptr);
    ~ReceptorModel();

    void save(std::vector<ReceptorGroup>& groups) const;
    void load(const std::vector<ReceptorGroup>& groups);
    void addGroup(const ReceptorGroup& group);
    void addReceptor(const QModelIndex& parent, const ReceptorNode& node);
    void updateRing(const QModelIndex& index, double buffer, double spacing, std::shared_ptr<SourceGroup> sg);
    void updateGrid(const QModelIndex& index, double xInit, double yInit, int xCount, int yCount, double xDelta, double yDelta);
    void updateZElev(const QModelIndex& index, double zElev);
    void updateZHill(const QModelIndex& index, double zHill);
    void updateZFlag(const QModelIndex& index, double zFlag);
    ReceptorGroup& groupFromIndex(const QModelIndex& index);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& index = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    std::vector<ReceptorGroup> localData;
};

#endif // RECEPTORMODEL_H
