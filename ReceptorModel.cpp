#include "ReceptorModel.h"
#include "ReceptorVisitor.h"

#include <QApplication>
#include <QFont>
#include <QIcon>
#include <QPalette>
#include <QString>
#include <QVariant>
#include <QDebug>

#include <map>
#include <memory>
#include <vector>

#include <boost/variant.hpp>

ReceptorModel::ReceptorModel(QObject *parent)
    : QAbstractItemModel(parent)
{}

ReceptorModel::~ReceptorModel()
{}

void ReceptorModel::save(std::vector<ReceptorGroup>& groups) const
{
    groups.clear();
    groups.reserve(localData.size());
    for (const auto& group : localData) {
        groups.push_back(group);
    }
}

void ReceptorModel::load(const std::vector<ReceptorGroup>& groups)
{
    beginResetModel();

    localData.clear();
    localData.reserve(groups.size());
    for (const auto& group : groups) {
        localData.push_back(group);
    }

    endResetModel();
}

void ReceptorModel::addGroup(const ReceptorGroup& group)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    localData.push_back(group);
    endInsertRows();
}

void ReceptorModel::addReceptor(const QModelIndex& parent, const ReceptorNode& node)
{
    ReceptorGroup& variant = groupFromIndex(parent);
    if (variant.which() != 0)
        return;

    ReceptorNodeGroup& group = boost::get<ReceptorNodeGroup>(variant);

    // Do nothing if node already exists.
    auto it = group.nodes.find(node);
    if (it != group.nodes.end())
        return;

    // Determine where the node will be inserted.
    int row = rowCount(parent);
    auto lower = group.nodes.lower_bound(node);
    if (lower != group.nodes.end())
        row = std::distance(group.nodes.begin(), lower);

    // Insert the new node.
    beginInsertRows(parent, row, row);
    group.nodes.emplace_hint(lower, node);
    endInsertRows();
}

void ReceptorModel::updateRing(const QModelIndex& index, double buffer, double spacing, std::shared_ptr<SourceGroup> sg)
{
    ReceptorGroup& variant = groupFromIndex(index);
    if (variant.which() != 1)
        return;

    int n = rowCount(index);
    removeRows(0, n, index);

    ReceptorRingGroup& ring = boost::get<ReceptorRingGroup>(variant);
    ring.setBuffer(buffer);
    ring.setSpacing(spacing);
    ring.setSourceGroup(sg);

    if (ring.updateGeometry()) {
        n = static_cast<int>(ring.nodeCount());
        beginInsertRows(index, 0, n - 1);
        endInsertRows();
    }
}

void ReceptorModel::updateGrid(const QModelIndex& index, double xInit, double yInit, int xCount, int yCount, double xDelta, double yDelta)
{
    ReceptorGroup& variant = groupFromIndex(index);
    if (variant.which() != 2)
        return;

    int n = rowCount(index);
    removeRows(0, n, index);

    ReceptorGridGroup& grid = boost::get<ReceptorGridGroup>(variant);
    if (grid.setDimensions(xCount, yCount) &&
        grid.setOrigin(xInit, yInit) &&
        grid.setSpacing(xDelta, yDelta))
    {
        n = static_cast<int>(grid.nodeCount());
        beginInsertRows(index, 0, n - 1);
        endInsertRows();
    }
}

void ReceptorModel::updateZElev(const QModelIndex& index, double zElev)
{
    if (index.internalId() == 0)
        return;

    ReceptorGroup& group = groupFromIndex(index);
    boost::apply_visitor(SetReceptorZElev(index.row(), zElev), group);
    QModelIndex zElevIndex = index.siblingAtColumn(Column::Z);
    emit dataChanged(zElevIndex, zElevIndex);
}

void ReceptorModel::updateZHill(const QModelIndex& index, double zHill)
{
    if (index.internalId() == 0)
        return;

    ReceptorGroup& group = groupFromIndex(index);
    boost::apply_visitor(SetReceptorZHill(index.row(), zHill), group);
    QModelIndex zHillIndex = index.siblingAtColumn(Column::ZHill);
    emit dataChanged(zHillIndex, zHillIndex);
}

void ReceptorModel::updateZFlag(const QModelIndex& index, double zFlag)
{
    if (index.internalId() == 0)
        return;

    ReceptorGroup& group = groupFromIndex(index);
    boost::apply_visitor(SetReceptorZFlag(index.row(), zFlag), group);
    QModelIndex zFlagIndex = index.siblingAtColumn(Column::ZFlag);
    emit dataChanged(zFlagIndex, zFlagIndex);
}

ReceptorGroup& ReceptorModel::groupFromIndex(const QModelIndex& index)
{
    int groupIndex = (index.internalId() == 0) ?
        index.row() : index.parent().row();

    return localData.at(groupIndex);
}

QModelIndex ReceptorModel::index(int row, int column, const QModelIndex& parent) const
{
    // Parent internal ID is always zero.
    if (!parent.isValid()) {
        return createIndex(row, column, (quintptr) 0);
    }

    // Child internal ID is the parent row (1-based).
    return createIndex(row, column, (quintptr)(parent.row() + 1));
}

QModelIndex ReceptorModel::parent(const QModelIndex &index) const
{
    if (index.isValid() && index.internalId() > 0) {
        int parentRow = static_cast<int>(index.internalId() - 1);
        return createIndex(parentRow, 0, (quintptr) 0);
    }

    return QModelIndex();
}

int ReceptorModel::rowCount(const QModelIndex &parent) const
{
    if (localData.empty())
        return 0;

    if (!parent.isValid())
        return static_cast<int>(localData.size());

    if (parent.internalId() == 0) {
        const ReceptorGroup& group = localData.at(parent.row());
        std::size_t n = boost::apply_visitor(ReceptorNodeCountVisitor(), group);
        return static_cast<int>(n);
    }

    return 0;
}

int ReceptorModel::columnCount(const QModelIndex &parent) const
{
    return 8;
}

bool ReceptorModel::hasChildren(const QModelIndex &index) const
{
    if (index.internalId() == 0) {
        return true;
    }
    return false;
}

QVariant ReceptorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.internalId() != 0 && !index.parent().isValid()))
        return QVariant();

    if (localData.empty())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (index.internalId() == 0) {
            if (index.column() != Column::Color &&
                index.column() != Column::Group &&
                index.column() != Column::Type) {
                return QVariant();
            }

            int groupIndex = index.row();
            auto& group = localData.at(groupIndex);

            switch (index.column()) {
            case Column::Color:
                return boost::apply_visitor(ReceptorGroupColorVisitor(), group);
            case Column::Group:
                return QString::fromStdString(boost::apply_visitor(ReceptorGroupNameVisitor(), group));
            case Column::Type:
                return boost::apply_visitor(ReceptorGroupTypeVisitor(), group);
            default: return QVariant();
            }
        }
        else {
            if (index.column() != Column::X &&
                index.column() != Column::Y &&
                index.column() != Column::Z &&
                index.column() != Column::ZHill &&
                index.column() != Column::ZFlag) {
                return QVariant();
            }

            int groupIndex = index.parent().row();
            auto& group = localData.at(groupIndex);
            auto visitor = ReceptorGroupNodeVisitor(index.row());
            auto node = boost::apply_visitor(visitor, group);

            switch (index.column()) {
            case Column::X:
                return node.x;
            case Column::Y:
                return node.y;
            case Column::Z:
                return node.zElev;
            case Column::ZHill:
                return node.zHill;
            case Column::ZFlag:
                return node.zFlag;
            default: return QVariant();
            }
        }
    }
    else if (role == Qt::BackgroundColorRole)
    {
        return (index.internalId() == 0) ?
            QApplication::palette().color(QPalette::Window) :
            QApplication::palette().color(QPalette::Base);
    }

    return QVariant();
}

bool ReceptorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (index.internalId() != 0 && !index.parent().isValid()))
        return false;

    if (localData.empty())
        return false;

    if (role == Qt::EditRole)
    {
        if (index.column() != Column::Color &&
            index.column() != Column::Group) {
            return false;
        }

        auto& group = localData.at(index.row());

        if (index.internalId() == 0)
        {
            if (index.column() == Column::Color) {
                QColor color = qvariant_cast<QColor>(value);
                boost::apply_visitor(SetReceptorGroupColor(color), group);
                emit dataChanged(index, index);
                return true;
            }
            else if (index.column() == Column::Group) {
                std::string grpid = value.toString().toUpper().toStdString();
                boost::apply_visitor(SetReceptorGroupName(grpid), group);
                emit dataChanged(index, index);
                return true;
            }
        }
    }

    return false;
}

QVariant ReceptorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:  return QString();
                case 1:  return QString("Group");
                case 2:  return QString("Type");
                case 3:  return QString("X");
                case 4:  return QString("Y");
                case 5:  return QString("Z");
                case 6:  return QString("ZHill");
                case 7:  return QString("ZFlag");
                default: return QVariant();
            }
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        if (orientation == Qt::Horizontal) {
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    return QVariant();
}

Qt::ItemFlags ReceptorModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || (index.internalId() != 0 && !index.parent().isValid()))
        return Qt::NoItemFlags;

    if (localData.empty())
        return Qt::NoItemFlags;

    if (index.internalId() != 0 && index.parent().row() >= localData.size())
        return Qt::NoItemFlags;

    // Group name is editable.
    if (index.internalId() == 0 && index.column() == Column::Group)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    // Children are not editable directly. Qt::ItemNeverHasChildren is an optimization.
    if (index.internalId() > 0) {
        QFlags childFlags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren;
        return childFlags;
    }

    return QAbstractItemModel::flags(index);
}

bool ReceptorModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count < 1 || row + count > rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count - 1);

    if (!parent.isValid()) {
        // Remove receptor groups.
        auto it = localData.begin() + row;
        for (int i = 0; i < count; ++i)
            it = localData.erase(it);
    }
    else {
        // Remove receptors.
        auto& group = localData.at(parent.row());
        boost::apply_visitor(RemoveReceptorGroupNodes(row, count), group);
    }

    endRemoveRows();

    return true;
}
