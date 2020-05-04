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

#include <QAbstractItemModel>
#include <QString>

#include <map>
#include <vector>

namespace osgeo {
namespace proj {
namespace io {

class WKTNode;

} // namespace io
} // namespace proj
} // namespace osgeo

class WKTModel : public QAbstractItemModel
{
    using WKTNode = osgeo::proj::io::WKTNode;

public:
    explicit WKTModel(QObject *parent = nullptr);

    bool load(const std::string& wkt);

    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    std::unique_ptr<WKTNode> rootNode_;
    std::map<WKTNode *, WKTNode *> childToParent_;
};
