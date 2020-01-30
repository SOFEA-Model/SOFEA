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

#include <QWidget>
#include <QString>

#include <map>
#include <memory>
#include <vector>

#include "Scenario.h"
#include "ReceptorModel.h"
#include "ReceptorTreeView.h"
#include "StandardPlot.h"
#include "widgets/DoubleLineEdit.h"

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpacerItem;
class QSpinBox;
class QStackedWidget;
class QToolButton;
QT_END_NAMESPACE

class QwtPlotItem;
class ReceptorParamsEditor;
struct ReceptorParamsVisitor;

class GroupEditor : public QWidget
{
    Q_OBJECT

public:
    explicit GroupEditor(ReceptorTreeView *view, QWidget *parent = nullptr);
    void setDefault(bool);

private slots:
    void onEdit();
    void onAddClicked(bool);

private:
    ReceptorTreeView *view;
    QButtonGroup *optionGroup;
    QRadioButton *btnNode;
    QRadioButton *btnRing;
    QRadioButton *btnGrid;
    QPushButton *btnAdd;
    QLineEdit *leGroupName;
};


class NodeEditor : public QWidget
{
public:
    explicit NodeEditor(QWidget *parent = nullptr);

private:
    DoubleLineEdit *leNodeX;
    DoubleLineEdit *leNodeY;
    DoubleLineEdit *leNodeZ;
    DoubleLineEdit *leNodeZHill;
    DoubleLineEdit *leNodeZFlag;

    friend class ReceptorParamsEditor;
    friend struct ReceptorParamsVisitor;
};


class RingParamsEditor : public QWidget
{
public:
    explicit RingParamsEditor(QWidget *parent = nullptr);

private:
    QComboBox *cboSourceGroup;
    QDoubleSpinBox *sbBuffer;
    QDoubleSpinBox *sbSpacing;

    friend class ReceptorParamsEditor;
    friend struct ReceptorParamsVisitor;
};


class GridParamsEditor : public QWidget
{
public:
    explicit GridParamsEditor(QWidget *parent = nullptr);

private:
    DoubleLineEdit *leGridXInit;
    DoubleLineEdit *leGridYInit;
    QSpinBox *sbGridXCount;
    QSpinBox *sbGridYCount;
    QDoubleSpinBox *sbGridXDelta;
    QDoubleSpinBox *sbGridYDelta;

    friend class ReceptorParamsEditor;
    friend struct ReceptorParamsVisitor;
};


class ReceptorParamsEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ReceptorParamsEditor(ReceptorTreeView *view, QWidget *parent = nullptr);
    ~ReceptorParamsEditor();

    enum StateFlag {
        NoSelection = 0x1,
        InvalidSelection = 0x2,
        EditDiscreteGroup = 0x4,
        EditRingGroup = 0x8,
        EditGridGroup = 0x16,
        EditSingleDiscrete = 0x32,
        EditSingleRing = 0x64,
        EditSingleGrid = 0x128,
        EditMultipleDiscrete = 0x256,
        EditMultipleRing = 0x512,
        EditMultipleGrid = 0x1024
    };

    Q_DECLARE_FLAGS(State, StateFlag)

    void setDefault(bool);
    void load(Scenario *s);

private slots:
    void onRemoveClicked(bool);
    void onAddClicked(bool);
    void onUpdateClicked(bool);
    void onDataChanged(const QModelIndex& first, const QModelIndex& last, const QVector<int>&);
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);

private:
    void setupConnections();
    void resetControls();
    void resetHeader(const QString& grpid);

    ReceptorTreeView *view;
    std::vector<std::weak_ptr<SourceGroup>> sgPtrs;
    QModelIndexList selectedRows;
    StateFlag currentState = StateFlag::NoSelection;

    QLabel *headerLabel;
    QLabel *statusLabel;
    NodeEditor *nodeEditor;
    RingParamsEditor *ringEditor;
    GridParamsEditor *gridEditor;
    QStackedWidget *editorStack;
    QStackedWidget *buttonStack;
    //QPushButton *btnImport;
    QPushButton *btnAdd;
    QPushButton *btnRemove;
    QPushButton *btnUpdate;

    friend struct ReceptorParamsVisitor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ReceptorParamsEditor::State)

class ReceptorEditor : public QWidget
{
    Q_OBJECT

public:
    ReceptorEditor(QWidget *parent = nullptr);
    ~ReceptorEditor();

public slots:
    void save(Scenario *s);
    void load(Scenario *s);

private slots:
    void onModelReset();
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onRowsRemoved(const QModelIndex& parent, int first, int last);
    void onDataChanged(const QModelIndex& first, const QModelIndex& last, const QVector<int>&);

private:
    void setupConnections();

    ReceptorModel *model;
    ReceptorTreeView *view;
    GroupEditor *groupEditor;
    ReceptorParamsEditor *paramsEditor;
    StandardPlot *plot;

    struct PlotItemDeleterFunctor {
        void operator()(QwtPlotItem *p) {
            p->detach();
            delete p;
        }
    };

    using PlotItemPtr = std::unique_ptr<QwtPlotItem, PlotItemDeleterFunctor>;
    std::multimap<QModelIndex, PlotItemPtr> plotItems;
};
