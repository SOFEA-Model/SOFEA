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

#include <QObject>
#include <QFileDialog>
#include <QStyledItemDelegate>

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

class FilePickerDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FilePickerDelegate(QObject *parent = nullptr);

    QString dialogCaption() const;
    QString dialogDirectory() const;
    QString dialogFilter() const;
    QFileDialog::FileMode dialogFileMode() const;

    void setDialogCaption(const QString& caption);
    void setDialogDirectory(const QString& dir);
    void setDialogFilter(const QString& filter);
    void setDialogFileMode(QFileDialog::FileMode mode);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem& option, const QModelIndex& index);

private:
    QString dialogCaption_;
    QString dialogDirectory_;
    QString dialogFilter_;
    QFileDialog::FileMode dialogFileMode_ = QFileDialog::ExistingFile;
};
