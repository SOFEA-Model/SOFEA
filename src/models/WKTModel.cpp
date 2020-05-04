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

#include "WKTModel.h"

#include <proj/io.hpp>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <algorithm>
#include <functional>
#include <iterator>

using WKTNodeNNPtr = osgeo::proj::io::WKTNodeNNPtr;
using ParsingException = osgeo::proj::io::ParsingException;

WKTModel::WKTModel(QObject *parent)
    : QAbstractItemModel(parent)
{}

bool WKTModel::load(const std::string& wkt)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Projection");

    // Called recursively to build child to parent map.
    std::function<void(WKTNode *)> updateMap = [&](WKTNode *parent) {
        if (parent) {
            for (const WKTNodeNNPtr& child : parent->children()) {
                childToParent_[child.get()] = parent;
                updateMap(child.get());
            }
        }
    };

    bool rc;
    beginResetModel();
    try {
        rootNode_.release();
        rootNode_ = WKTNode::createFrom(wkt).as_nullable();
        updateMap(rootNode_.get());
        rc = true;
    } catch (const ParsingException& e) {
        BOOST_LOG_TRIVIAL(error) << "PROJ: " << e.what();
        rc = false;
    }
    endResetModel();
    return rc;
}

QModelIndex WKTModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    WKTNode *parentNode;
    if (!parent.isValid())
        parentNode = rootNode_.get();
    else
        parentNode = static_cast<WKTNode *>(parent.internalPointer());

    WKTNode *childNode = parentNode->children().at(row).get();
    if (childNode)
        return createIndex(row, column, childNode);
    else
        return QModelIndex();
}

QModelIndex WKTModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    WKTNode *node = static_cast<WKTNode *>(index.internalPointer());

    // Get internal pointer.
    auto it = childToParent_.find(node);
    WKTNode *parentNode = it != childToParent_.end() ? it->second : nullptr;

    if (parentNode == rootNode_.get())
        return QModelIndex();

    // Get parent node position.
    int pos = static_cast<int>(std::distance(childToParent_.begin(), it));

    return createIndex(pos, 0, parentNode);
}

QVariant WKTModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    WKTNode *node = static_cast<WKTNode *>(index.internalPointer());
    if (node == nullptr)
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return QString::fromStdString(node->value());
    }
    
    return QVariant();
}

QVariant WKTModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
    
    if (section == 0)
        return "Value";
    else
        return QVariant();
}

int WKTModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    WKTNode *parentNode;
    if (!parent.isValid())
        parentNode = rootNode_.get();
    else
        parentNode = static_cast<WKTNode *>(parent.internalPointer());

    if (parentNode == nullptr)
        return 0;

    return static_cast<int>(parentNode->children().size());
}

int WKTModel::columnCount(const QModelIndex &) const
{
    return 1;
}
