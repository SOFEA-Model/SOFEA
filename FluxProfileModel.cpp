#include "FluxProfileModel.h"

#include <QApplication>
#include <QMetaType>
#include <QSettings>
#include <QString>

#include <QDebug>

#include <fmt/format.h>

QString dsMethodString(FluxProfile::DSMethod method)
{
    switch (method) {
        case FluxProfile::DSMethod::Disabled:      return QString("Disabled");
        case FluxProfile::DSMethod::LinearCDPR:    return QString("Linear (CDPR)");
        case FluxProfile::DSMethod::LinearGeneral: return QString("Linear (General)");
        case FluxProfile::DSMethod::Nonlinear:     return QString("Nonlinear");
    }

    return QString();
}

QString tsMethodString(FluxProfile::TSMethod method)
{
    switch (method) {
        case FluxProfile::TSMethod::Disabled:      return QString("Disabled");
        case FluxProfile::TSMethod::Seasonal:      return QString("Seasonal (CDPR)");
        case FluxProfile::TSMethod::Sinusoidal:    return QString("Sinusoidal");
    }

    return QString();
}

FluxProfileModel::FluxProfileModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void FluxProfileModel::save(std::vector<std::shared_ptr<FluxProfile>> &profiles) const
{
    profiles = modelData;
}

void FluxProfileModel::load(const std::vector<std::shared_ptr<FluxProfile>>& profiles, bool copy)
{
    beginResetModel();

    // If copy is true, initialize local data with deep copy.
    // Otherwise, pointers refer to the original object.

    if (copy) {
        modelData.clear();
        modelData.reserve(profiles.size());
        for (const auto fp : profiles) {
            modelData.emplace_back(std::make_shared<FluxProfile>(*fp));
        }
    }
    else {
        modelData = profiles;
    }

    endResetModel();
}

void FluxProfileModel::reset()
{
    beginResetModel();
    endResetModel();
}

std::shared_ptr<FluxProfile> FluxProfileModel::fluxProfileFromIndex(const QModelIndex &index)
{
    return modelData.at(index.row());
}

int FluxProfileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return modelData.size();
}

int FluxProfileModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant FluxProfileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    auto fp = modelData.at(index.row());

    if (role == Qt::UserRole) {
        return QVariant();
    }

    if (role == Qt::EditRole) {
        switch (index.column()) {
            case 0:  return QString::fromStdString(fp->name);
            default: return QVariant();
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:  return QString::fromStdString(fp->name);
            case 1:  return fp->totalHours();
            case 2:  return tsMethodString(fp->tsMethod);
            case 3:  return dsMethodString(fp->dsMethod);
            default: return QVariant();
        }
    }

    return QVariant();
}

bool FluxProfileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.isValid())
    {
        auto fp = modelData.at(index.row());
        switch (index.column()) {
            case 0: {
                QString name = value.toString();
                // Remove whitespace and truncate to max length (100)
                name = name.simplified();
                name.truncate(100);
                fp->name = name.toStdString();
                break;
            }
            default:
                return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant FluxProfileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:  return QString("Profile");
                case 1:  return QString("Hours");
                case 2:  return QString("Time Scaling");
                case 3:  return QString("Depth Scaling");
                default: return QVariant();
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags FluxProfileModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    // Set editable flag for name column.
    switch (index.column()) {
        case 0:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        default: return QAbstractTableModel::flags(index);
    }
}

bool FluxProfileModel::insertRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || row > modelData.size())
        return false;

    auto it = modelData.begin() + row;
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i=0; i < count; ++i) {
        auto fp = std::make_shared<FluxProfile>();
        modelData.insert(it, fp);
    }
    endInsertRows();

    return true;
}

bool FluxProfileModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || row > modelData.size())
        return false;

    auto it = modelData.begin() + row;
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        it = modelData.erase(it);
    }
    endRemoveRows();

    return true;
}

bool FluxProfileModel::moveRows(const QModelIndex &, int sourceFirst, int count,
                                const QModelIndex &, int destinationFirst)
{
    if (count <= 0 || sourceFirst == destinationFirst)
        return false;

    int sourceLast = sourceFirst + count;
    int destinationLast = destinationFirst + count;

    if (sourceLast > modelData.size() || destinationLast > modelData.size())
        return false;

    // Get the extraction range.
    decltype(modelData) range(modelData.begin() + sourceFirst,
                              modelData.begin() + sourceLast);

    // Notify views of new state.
    // See documentation for QAbstractItemModel::beginMoveRows.
    int destinationChild = destinationFirst < sourceFirst ? destinationFirst
                                                          : destinationLast;
    beginMoveRows(QModelIndex(), sourceFirst, sourceLast - 1,
                  QModelIndex(), destinationChild);

    // Erase and insert.
    modelData.erase(modelData.begin() + sourceFirst,
                    modelData.begin() + sourceLast);

    modelData.insert(modelData.begin() + destinationFirst,
                     range.begin(), range.end());

    endMoveRows();

    return true;
}
