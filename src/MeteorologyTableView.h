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

#include "widgets/StandardTableView.h"

class MeteorologyModel;

QT_BEGIN_NAMESPACE
class QDropEvent;
QT_END_NAMESPACE

class MeteorologyTableView : public StandardTableView
{
    Q_OBJECT

public:
    MeteorologyTableView(MeteorologyModel *model, QWidget *parent = nullptr);

protected:
    void dropEvent(QDropEvent *e) override;

private:
    MeteorologyModel *model_ = nullptr;
};
