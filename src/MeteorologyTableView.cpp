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

#include "MeteorologyTableView.h"
#include "models/MeteorologyModel.h"

#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>

MeteorologyTableView::MeteorologyTableView(MeteorologyModel *model, QWidget *parent)
    : StandardTableView(parent), model_(model)
{
    setModel(model);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);

    QHeaderView *header = horizontalHeader();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(QHeaderView::Interactive);

    header->setSectionResizeMode(MeteorologyModel::Name,            QHeaderView::Stretch);
    header->setSectionResizeMode(MeteorologyModel::SurfaceStation,  QHeaderView::Fixed);
    header->setSectionResizeMode(MeteorologyModel::UpperAirStation, QHeaderView::Fixed);
    header->setSectionResizeMode(MeteorologyModel::OnSiteStation,   QHeaderView::Fixed);
    header->setSectionResizeMode(MeteorologyModel::StartTime,       QHeaderView::Fixed);
    header->setSectionResizeMode(MeteorologyModel::EndTime,         QHeaderView::Fixed);
    header->setSectionResizeMode(MeteorologyModel::TotalHours,      QHeaderView::Fixed);
    header->setSectionResizeMode(MeteorologyModel::CalmHours,       QHeaderView::Fixed);
    header->setSectionResizeMode(MeteorologyModel::MissingHours,    QHeaderView::Fixed);

    QFontMetrics fm = fontMetrics();
    int maxCharWidth = fm.horizontalAdvance(QChar('W'));
    int dateWidth = fm.horizontalAdvance("0000-00-00 00:00") + maxCharWidth * 2;

    header->resizeSection(MeteorologyModel::SurfaceStation,  maxCharWidth * 10);
    header->resizeSection(MeteorologyModel::UpperAirStation, maxCharWidth * 10);
    header->resizeSection(MeteorologyModel::OnSiteStation,   maxCharWidth * 10);
    header->resizeSection(MeteorologyModel::StartTime,       dateWidth);
    header->resizeSection(MeteorologyModel::EndTime,         dateWidth);
    header->resizeSection(MeteorologyModel::TotalHours,      maxCharWidth * 8);
    header->resizeSection(MeteorologyModel::CalmHours,       maxCharWidth * 8);
    header->resizeSection(MeteorologyModel::MissingHours,    maxCharWidth * 8);
}

void MeteorologyTableView::dropEvent(QDropEvent *e)
{
    if (!model_)
        return;

    const QMimeData* mimeData = e->mimeData();

    if (!mimeData->hasUrls())
        return;

    QList<QUrl> urls = mimeData->urls();
    model_->addUrls(urls);
}
