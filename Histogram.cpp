/**
 * @file   Histogram.cpp
 * @author Dan R. Lipsa
 * @date 22 July 2010
 *
 * Definition of the Histogram class
 */

#include "Histogram.h"

Histogram::Histogram (QWidget* parent) :
    QwtPlot (parent)
{
    setCanvasBackground(QColor(Qt::white));
    setTitle("Histogram");

    m_grid.enableXMin(true);
    m_grid.enableYMin(true);
    m_grid.setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    m_grid.setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    m_grid.attach(this);

    m_histogramItem.setColor(Qt::darkCyan);
    m_histogramItem.attach(this);
}


void Histogram::SetData (const QwtIntervalData& intervalData)
{
    m_histogramItem.setData(intervalData);
    double maxValue = numeric_limits<double>().min ();
    for (size_t i = 0; i < intervalData.size (); ++i)
	maxValue = max (maxValue, intervalData.value (i));

    setAxisScale(QwtPlot::yLeft, 0.0, double(maxValue));
    setAxisScale(
	QwtPlot::xBottom,
	intervalData.interval (0).minValue (),
	intervalData.interval (intervalData.size () - 1).maxValue ());
}
