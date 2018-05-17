#include "SourceModel.h"
#include "RunstreamParser.h"

#include <QIcon>

#include <boost/ptr_container/clone_allocator.hpp>
#include "fmt/format.h"

SourceModel::SourceModel(SourceGroup *sg, QObject *parent)
    : QAbstractTableModel(parent), sgPtr(sg)
{}

void SourceModel::setDirectEditMode(bool on)
{
    directEdit = on;
}

void SourceModel::save()
{
    if (directEdit)
        return; // already saved.

    // Sources will eventually be deallocated.
    sgPtr->sources.release();
    sgPtr->sources = sources.clone();
    sgPtr->resetGeometry();
}

void SourceModel::load()
{
    beginResetModel();
    sources = sgPtr->sources.clone();
    endResetModel();
}

void SourceModel::reset()
{
    beginResetModel();
    endResetModel();
}

void SourceModel::import(const QString &file)
{
    beginResetModel();
    boost::ptr_vector<Source> imported;
    RunstreamParser::parseSources(file, imported);
    for (Source &s : imported)
        sgPtr->initSource(&s);

    sources.transfer(sources.end(), imported.begin(), imported.end(), imported);
    endResetModel();
}

Source* SourceModel::getSource(const QModelIndex &index)
{
    // If directEdit==true, show data from underlying datastore.
    Source &s = directEdit ? sgPtr->sources.at(index.row())
                           : sources.at(index.row());
    return &s;
}

void SourceModel::addAreaSource()
{
    static unsigned int seqArea = 1;
    AreaSource *s = new AreaSource;
    s->srcid = fmt::format("Area{:0=4}", seqArea);
    s->setGeometry();
    sgPtr->initSource(s);
    seqArea++;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    if (directEdit)
        sgPtr->sources.push_back(s);
    else
        sources.push_back(s);
    endInsertRows();
}

void SourceModel::addAreaCircSource()
{
    static unsigned int seqCirc = 1;
    AreaCircSource *s = new AreaCircSource;
    s->srcid = fmt::format("Circ{:0=4}", seqCirc);
    s->setGeometry();
    sgPtr->initSource(s);
    seqCirc++;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    if (directEdit)
        sgPtr->sources.push_back(s);
    else
        sources.push_back(s);
    endInsertRows();
}

void SourceModel::addAreaPolySource()
{
    static unsigned int seqPoly = 1;
    AreaPolySource *s = new AreaPolySource;
    s->srcid = fmt::format("Poly{:0=4}", seqPoly);

    QPointF v1(s->xs, s->ys);
    QPointF v2(s->xs, s->ys + 100);
    QPointF v3(s->xs + 100, s->ys + 100);
    QPointF v4(s->xs + 100, s->ys);
    s->geometry << v1 << v2 << v3 << v4 << v1;
    sgPtr->initSource(s);
    seqPoly++;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    if (directEdit)
        sgPtr->sources.push_back(s);
    else
        sources.push_back(s);
    endInsertRows();
}

int SourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (directEdit)
        return sgPtr->sources.size();
    else
        return sources.size();
}

int SourceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 11;
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    // If directEdit==true, show data from underlying datastore.
    const Source &s = directEdit ? sgPtr->sources.at(index.row())
                                 : sources.at(index.row());

    if (role == Qt::UserRole) {
        return QVariant();
    }

    if (role == Qt::EditRole) {
        switch (index.column()) {
            case 0:  return QString::fromStdString(s.srcid);
            case 4:  return s.appStart;
            case 5:  return s.appRate;
            case 6:  return s.incorpDepth;
            default: return QVariant();
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:  return QString::fromStdString(s.srcid);
            case 1:  return s.xs;
            case 2:  return s.ys;
            case 3:  return s.area();
            case 4:  return s.appStart.toString("yyyy-MM-dd HH:mm");
            case 5:  return s.appRate;
            case 6:  return s.incorpDepth;
            case 7:  return s.appRate * s.area() * sgPtr->appFactor;
            case 8:  return sgPtr->fluxScaling.timeScaleFactor(s.appStart);
            case 9:  return sgPtr->fluxScaling.depthScaleFactor(s.incorpDepth);
            case 10: return sgPtr->fluxScaling.fluxScaleFactor(s.appRate, s.appStart, s.incorpDepth);
        }
    }

    if (role == Qt::ForegroundRole) {
        // Monte Carlo parameters: blue
        // Calculated parameters: red
        switch (index.column()) {
            case 0:  return QVariant();
            case 1:  return QVariant();
            case 2:  return QVariant();
            case 3:  return QVariant(QColor(Qt::red));
            case 4:  return QVariant(QColor(Qt::blue));
            case 5:  return QVariant(QColor(Qt::blue));
            case 6:  return QVariant(QColor(Qt::blue));
            case 7:  return QVariant(QColor(Qt::red));
            case 8:  return QVariant(QColor(Qt::red));
            case 9:  return QVariant(QColor(Qt::red));
            case 10: return QVariant(QColor(Qt::red));
        }
    }

    return QVariant();
}

bool SourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.isValid()) {
        int row = index.row();

        // If directEdit==true, commit changes directly to underlying datastore,
        // without requiring save().
        Source &s = directEdit ? sgPtr->sources.at(row)
                               : sources.at(row);

        switch (index.column()) {
            case 0: {
                QString srcid = value.toString();
                srcid.truncate(8);
                s.srcid = srcid.toStdString();
                break;
            }
            case 4: {
                QDateTime appStart = value.toDateTime();
                s.appStart = appStart;
                break;
            }
            case 5: {
                double appRate = value.toDouble();
                s.appRate = appRate;
                break;
            }
            case 6: {
                double incorpDepth = value.toDouble();
                s.incorpDepth = incorpDepth;
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

QVariant SourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:  return QString("ID");
                case 1:  return QString("X (m)");
                case 2:  return QString("Y (m)");
                case 3:  return QString("Area (ha)");
                case 4:  return QString("Start");
                case 5:  return QString("App. Rate (kg/ha)");
                case 6:  return QString("Inc. Depth (cm)");
                case 7:  return QLatin1String("Mass \xd7 AF (kg)");
                case 8:  return QString("Time SF");
                case 9:  return QString("Depth SF");
                case 10: return QString("Overall SF");
                default: return QVariant();
            }
        }
    }

    if (role == Qt::DecorationRole) {
        if (orientation == Qt::Vertical) {
            if (section >= rowCount() || section < 0)
                return QVariant();

            const Source &s = directEdit ? sgPtr->sources.at(section)
                                         : sources.at(section);

            switch (s.getType()) {
                case SourceType::AREA:
                    return QIcon(":/images/Rectangle_16x.png");
                case SourceType::AREACIRC:
                    return QIcon(":/images/Circle_16x.png");
                case SourceType::AREAPOLY:
                    return QIcon(":/images/Polygon_16x.png");
                default:
                    return QVariant();
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags SourceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    // Set editable flag for selected columns.
    switch (index.column()) {
        case 0:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 4:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 5:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 6:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        default: return QAbstractTableModel::flags(index);
    }
}

bool SourceModel::removeRows(int row, int count, const QModelIndex &)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);

    if (directEdit) {
        auto it = sgPtr->sources.begin() + row;
        for (int i = 0; i < count; ++i)
            it = sgPtr->sources.erase(it);
    } else {
        auto it = sources.begin() + row;
        for (int i = 0; i < count; ++i)
            it = sources.erase(it);
    }

    endRemoveRows();

    return true;
}
