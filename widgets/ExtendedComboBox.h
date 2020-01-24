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

#include <QComboBox>
#include <QList>

QT_BEGIN_NAMESPACE
class QFrame;
class QWidget;
QT_END_NAMESPACE

class ExtendedComboBox : public QComboBox
{
public:
    explicit ExtendedComboBox(QWidget *parent = nullptr);
    void setPopupHeight(int height);
    int popupHeight() const;

protected:
    virtual void showPopup() override;

private:
    QFrame *container_ = nullptr;
    QList<QWidget *> scrollers_;
    int popupHeight_ = 0;
};
