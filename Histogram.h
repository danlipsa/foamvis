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

private:
    QwtPlotGrid m_grid;
    HistogramItem m_histogramItem;
};


#endif //__HISTOGRAM_H__

// Local Variables:
// mode: c++
// End:
