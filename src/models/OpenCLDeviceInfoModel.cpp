// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "OpenCLDeviceInfoModel.h"

#include <boost/compute/system.hpp>
#include <boost/compute/exception/opencl_error.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <QCoreApplication>
#include <QString>

namespace compute = boost::compute;

QString deviceTypeString(enum compute::device::type deviceType)
{
    switch (deviceType) {
    case CL_DEVICE_TYPE_CPU:         return "CPU";
    case CL_DEVICE_TYPE_GPU:         return "GPU";
    case CL_DEVICE_TYPE_ACCELERATOR: return "Accelerator";
    default:                         return "Unknown";
    }
}

OpenCLDeviceInfoModel::OpenCLDeviceInfoModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

int OpenCLDeviceInfoModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(compute::system::device_count());
}

int OpenCLDeviceInfoModel::columnCount(const QModelIndex &) const
{
    return 7;
}

QVariant OpenCLDeviceInfoModel::data(const QModelIndex &index, int role) const
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "General");

    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    static auto devices = compute::system::devices();

    if (role == Qt::DisplayRole) {
        try {
            auto& device = devices.at(index.row());
            switch (index.column()) {
            case Name:
                return QString::fromStdString(device.name());
            case Type:
                switch (device.type()) {
                case compute::device::cpu:         return "CPU";
                case compute::device::gpu:         return "GPU";
                case compute::device::accelerator: return "Accelerator";
                default:                           return "Unknown";
                }
            case Version:
                return QString::fromStdString(device.version());
            case ComputeUnits:
                return device.compute_units();
            case ClockFrequency:
                return QString("%1 MHz").arg(device.clock_frequency());
            case GlobalMemorySize:
                return QString("%1 MB").arg(device.global_memory_size() / 1024 / 1024);
            case MaxImageSize: {
                int w = (int)device.get_info<std::size_t>(CL_DEVICE_IMAGE2D_MAX_WIDTH);
                int h = (int)device.get_info<std::size_t>(CL_DEVICE_IMAGE2D_MAX_HEIGHT);
                return QStringLiteral("%1 \u00d7 %3").arg(w).arg(h);
            }
            default: return QVariant();
            }
        } catch (const compute::opencl_error& ex) {
            BOOST_LOG_TRIVIAL(warning) << "OpenCL: " << ex.what();
            return QVariant();
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case Name:             return Qt::AlignLeft;
        case Type:             return Qt::AlignCenter;
        case Version:          return Qt::AlignLeft;
        case ComputeUnits:     return Qt::AlignCenter;
        case ClockFrequency:   return Qt::AlignCenter;
        case GlobalMemorySize: return Qt::AlignCenter;
        case MaxImageSize:     return Qt::AlignCenter;
        default: return QVariant();
        }
    }

    return QVariant();
}

QVariant OpenCLDeviceInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (section) {
        case Name:             return tr("Name");
        case Type:             return tr("Type");
        case Version:          return tr("Version");
        case ComputeUnits:     return tr("Compute Units");
        case ClockFrequency:   return tr("Clock Frequency");
        case GlobalMemorySize: return tr("Memory");
        case MaxImageSize:     return tr("Max Image Size");
        default: return QVariant();
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        switch (section) {
        case Name:             return Qt::AlignLeft;
        case Type:             return Qt::AlignCenter;
        case Version:          return Qt::AlignLeft;
        case ComputeUnits:     return Qt::AlignCenter;
        case ClockFrequency:   return Qt::AlignCenter;
        case GlobalMemorySize: return Qt::AlignCenter;
        case MaxImageSize:     return Qt::AlignCenter;
        default: return QVariant();
        }
    }

    return QVariant();
}

Qt::ItemFlags OpenCLDeviceInfoModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index);
}
