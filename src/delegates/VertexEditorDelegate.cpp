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

#include <QDoubleSpinBox>
#include <QPolygonF>
#include <QSignalBlocker>
#include <QSpinBox>

#include "VertexEditorDelegate.h"
#include "../models/SourceModel.h"
#include "../widgets/VertexEditor.h"

VertexEditorDelegate::VertexEditorDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QWidget* VertexEditorDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(option)

    VertexEditor *e = new VertexEditor(parent);
    return e;
}

void VertexEditorDelegate::setEditorData(QWidget *editor,
    const QModelIndex& index) const
{
    VertexEditor *e = qobject_cast<VertexEditor *>(editor);
    SourceModel *m = const_cast<SourceModel *>(
        qobject_cast<const SourceModel *>(index.model()));

    if (e == nullptr || m == nullptr) {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    {
        QModelIndex vertexCountIndex = m->index(index.row(), SourceModel::Column::VertexCount);
        int vertexCount = qvariant_cast<int>(m->data(vertexCountIndex, Qt::DisplayRole));

        if (vertexCount > 25) {
            // Imported source may have more than 25 vertices.
            // Set the maximum vertex count to actual.
            e->sbVertexCount->setMaximum(vertexCount);
        }
        else {
            // Maximum 25 vertices for a user defined source.
            e->sbVertexCount->setMaximum(25);
        }

        e->sbVertexCount->setValue(vertexCount);
        e->sbVertex->setMaximum(vertexCount);

        // Set controls to first vertex by default.
        QModelIndex firstVertexIndex = m->vertexIndex(index.row(), 0);
        const QPointF firstVertex = qvariant_cast<QPointF>(m->data(firstVertexIndex, Qt::UserRole));

        e->sbVertex->setValue(1);
        e->sbXCoord->setValue(firstVertex.x());
        e->sbYCoord->setValue(firstVertex.y());

        const_cast<VertexEditorDelegate *>(this)->emitCommitData();
    }

    connect(e, &VertexEditor::vertexCountChanged, [=](int count) {
        // Set controls to last vertex on vertex count decrease.
        if (count < e->sbVertex->value())
            e->sbVertex->setValue(count);
        e->sbVertex->setMaximum(count);
    });

    connect(e, &VertexEditor::vertexChanged, [=](int i) {
        // Set coordinates to current vertex.
        QModelIndex vertexIndex = m->vertexIndex(index.row(), i - 1);
        const QPointF vertex = qvariant_cast<QPointF>(m->data(vertexIndex, Qt::UserRole));

        e->sbXCoord->setValue(vertex.x());
        e->sbYCoord->setValue(vertex.y());
    });
}

void VertexEditorDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model,
    const QModelIndex& index) const
{
    VertexEditor *e = qobject_cast<VertexEditor *>(editor);
    SourceModel *m = qobject_cast<SourceModel *>(model);

    if (e == nullptr || m == nullptr) {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    int count = e->sbVertexCount->value();
    int vertex = e->sbVertex->value() - 1;
    double x = e->sbXCoord->value();
    double y = e->sbYCoord->value();
    QPointF p{x, y};

    QModelIndex vertexCountIndex = m->index(index.row(), SourceModel::Column::VertexCount);
    m->setData(vertexCountIndex, count, Qt::DisplayRole);

    QModelIndex vertexIndex = m->vertexIndex(index.row(), vertex);
    m->setData(vertexIndex, p, Qt::UserRole);

    const_cast<VertexEditorDelegate *>(this)->emitCommitData();
}

bool VertexEditorDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)

    emit editRequested();
    return false;
}

void VertexEditorDelegate::emitCommitData()
{
    // Notify other application components of the change.
    emit commitData(qobject_cast<QWidget *>(sender()));
}

