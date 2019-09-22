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
