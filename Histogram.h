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
    void SetSelected (bool selected);
    size_t GetBin (float value);

public Q_SLOTS:
    void Moved (const QPoint& pos);
    void Appended (const QPoint& pos);
    void Selected (const QwtPolygon& pos);

private:
    

private:
    Q_OBJECT

    QwtPlotGrid m_grid;
    HistogramItem m_histogramItem;
    QwtPlotPicker m_plotPicker;
    QPoint m_lastPos;
    QVector<bool> m_selected;
    size_t m_beginSelection;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
