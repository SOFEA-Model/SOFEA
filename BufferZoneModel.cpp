#include "BufferZoneModel.h"

#include <iterator>

BufferZoneModel::BufferZoneModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void BufferZoneModel::save(std::set<BufferZone>& zones) const
{
    zones = m_zones;
}

void BufferZoneModel::load(const std::set<BufferZone>& zones)
{
    beginResetModel();
    m_zones = zones;
    endResetModel();
}

void BufferZoneModel::insert(const BufferZone& zone)
{
    // Do nothing if buffer zone exists.
    auto it = m_zones.find(zone);
    if (it != m_zones.end())
        return;

    // Determine where the row will be inserted. O(n) complexity.
    int index = rowCount();
    auto lower = m_zones.lower_bound(zone);
    if (lower != m_zones.end())
        index = std::distance(m_zones.begin(), lower);

    // Insert the new row.
    beginInsertRows(QModelIndex(), index, index);
    m_zones.insert(zone);
    endInsertRows();
}

int BufferZoneModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_zones.size());
}

int BufferZoneModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant BufferZoneModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    BufferZone z = *std::next(m_zones.begin(), index.row());

    if (role == Qt::UserRole) {
        return QVariant();
    }

    if (role == Qt::EditRole) {
        switch (index.column()) {
            case 2:  return z.distance;
            case 3:  return z.duration;
            default: return QVariant();
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:  return z.areaThreshold;
            case 1:  return z.appRateThreshold;
            case 2:  return z.distance;
            case 3:  return z.duration;
            default: return QVariant();
        }
    }

    return QVariant();
}

bool BufferZoneModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.isValid() && index.row() < rowCount())
    {
        auto it = std::next(m_zones.begin(), index.row());
        switch (index.column()) {
            case 2:
                it->distance = value.toDouble();
                break;
            case 3:
                it->duration = value.toDouble();
                break;
            default:
                return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant BufferZoneModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:  return QString("Area (ha)");
                case 1:  return QString("App. Rate (kg/ha)");
                case 2:  return QString("Distance (m)");
                case 3:  return QString("Duration (hr)");
                default: return QVariant();
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags BufferZoneModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    // Set editable flag for name column.
    switch (index.column()) {
        case 2:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 3:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        default: return QAbstractTableModel::flags(index) & (~Qt::ItemIsEditable) & (~Qt::ItemIsEnabled);
    }
}

bool BufferZoneModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || row > rowCount())
        return false;

    auto it = std::next(m_zones.begin(), row);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        it = m_zones.erase(it);
    }
    endRemoveRows();

    return true;
}
