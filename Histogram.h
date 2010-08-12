/**
 * @file   Histogram.h
 * @author Dan R. Lipsa
 * @date 22 July 2010
 *
 * Declaration of the Histogram class
 */

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include "histogram_item.h"

class Histogram : public QwtPlot
{
public:
    Histogram (QWidget* parent = 0);
    void SetData (const QwtIntervalData& intervalData);

public Q_SLOTS:
    void Moved (const QPoint& pos);
    void Appended (const QPoint& pos);
    void Selected (const QwtPolygon& pos);

private:
    Q_OBJECT

    QwtPlotGrid m_grid;
    HistogramItem m_histogramItem;
    QwtPlotPicker m_plotPicker;
    QPoint m_lastPos;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
