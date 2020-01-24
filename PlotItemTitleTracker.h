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

#ifndef PLOTITEMTITLETRACKER_H
#define PLOTITEMTITLETRACKER_H

#include <qwt_plot_picker.h>

class QwtPlotShapeItem;

class PlotItemTitleTracker : public QwtPlotPicker
{
public:
    PlotItemTitleTracker(QWidget *canvas);

protected:
    virtual QwtText trackerText(const QPoint &) const override;

private:
    QwtPlotShapeItem* itemAt(const QPoint &) const;
};

#endif // PLOTITEMTITLETRACKER_H
