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

#include <future>
#include <memory>
#include <vector>

#include "core/Projection.h"
#include "core/Raster.h"
#include "core/Receptor.h"
#include "core/TaskControl.h"

class BoundingBoxEditor;
class OpenCLDeviceInfoModel;
class PathEdit;
class ProgressBar;
class ReadOnlyLineEdit;
class StatusLabel;

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRectF;
class QTreeView;
QT_END_NAMESPACE

class ReceptorElevationEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ReceptorElevationEditor(QWidget *parent = nullptr);

    void setCompoundCRS(std::shared_ptr<PJ> ccrs_);
    void setAreaOfUse(const Projection::GeographicExtent& area);
    void setReceptors(const std::vector<ReceptorGroup>& receptors);
    void setBoundingBox(const QRectF& bbox);

private slots:
    void onBoundingBoxChanged(const QRectF& bbox);
    void onStartStopClicked();
    void onStarted();
    void onProgress(double complete);
    void onMessage(const QString& message);
    void onFinished();

signals:
    void started();
    void progress(double complete);
    void message(const QString& text);
    void finished();

private:
    std::shared_ptr<PJ> ccrs_;
    Projection::GeographicExtent validArea_;
    std::unique_ptr<Raster::Dataset> dataset_;
    TaskControl control_;
    std::future<std::vector<float>> future_;

    StatusLabel *infoLabel;
    BoundingBoxEditor *bboxEdit;
    OpenCLDeviceInfoModel *deviceModel;
    QTreeView *deviceView;
    QCheckBox *outputCheckBox;
    PathEdit *outputPathEdit;
    StatusLabel *areaWarningLabel;
    QLabel *progressLabel;
    ProgressBar *progressBar;
    QPushButton *btnStartStop;
};
