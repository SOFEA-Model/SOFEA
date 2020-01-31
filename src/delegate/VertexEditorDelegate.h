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

#pragma once

#include <QStyledItemDelegate>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QDoubleSpinBox;
class QSpinBox;
QT_END_NAMESPACE

class VertexEditorDelegate;

class VertexEditor : public QWidget
{
    Q_OBJECT

friend class VertexEditorDelegate;

public:
    VertexEditor(QWidget *parent = nullptr);

signals:
    void geometryChanged();
    void vertexCountChanged(int i);
    void vertexChanged(int i);
    void coordinatesChanged(double);

private:
    QSpinBox *sbVertexCount;
    QSpinBox *sbVertex;
    QDoubleSpinBox *sbXCoord;
    QDoubleSpinBox *sbYCoord;
};

class VertexEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    VertexEditorDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget *editor,
        QAbstractItemModel *model, const QModelIndex& index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem& option, const QModelIndex& index);

signals:
    void editRequested();

private slots:
    void emitCommitData();
};