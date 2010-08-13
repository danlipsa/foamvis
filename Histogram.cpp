/**
 * @file   Histogram.cpp
 * @author Dan R. Lipsa
 * @date 22 July 2010
 *
 * Definition of the Histogram class
 */

#include "Histogram.h"
#include "DebugStream.h"
#include "BodySetStatistics.h"

Histogram::Histogram (QWidget* parent) :
    QwtPlot (parent), 
    m_plotPicker (QwtPlot::xBottom, QwtPlot::yLeft,
		  QwtPicker::RectSelection | QwtPicker::DragSelection, 
		  QwtPlotPicker::NoRubberBand,
		  QwtPicker::AlwaysOff,
		  canvas()),
    m_selectionTool (ERASER)
{
    setCanvasBackground(QColor(Qt::white));
    setAutoReplot ();

    m_grid.enableXMin(true);
    m_grid.enableYMin(true);
    m_grid.setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    m_grid.setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    m_grid.attach(this);

    m_histogramItem.setFocusColor(Qt::darkCyan);
    m_histogramItem.setContextColor(Qt::lightGray);

    m_histogramItem.attach(this);

    m_plotPicker.setRubberBandPen(QColor(Qt::green));
    m_plotPicker.setEnabled (false);

    connect(&m_plotPicker, SIGNAL(appended(const QPoint&)),
	    this, SLOT(SelectionPointAppended(const QPoint&)));
    connect(&m_plotPicker, SIGNAL(moved(const QPoint&)),
	    this, SLOT(SelectionPointMoved(const QPoint&)));
    connect(&m_plotPicker, SIGNAL(selected(const QwtPolygon&)),
	    this, SLOT(PolygonSelected (const QwtPolygon&)));
}

size_t Histogram::getBin (float value)
{
    const QwtIntervalData& data = m_histogramItem.data ();
    size_t binCount = data.size ();
    return BodySetStatistics::GetBin (
	value, binCount,
	data.interval (0).minValue (), data.interval (binCount - 1).maxValue ());
}

void Histogram::SelectionPointAppended (const QPoint &pos)
{
    double value = invTransform(QwtPlot::xBottom, pos.x());
    m_beginBinSelection = getBin (value);
    SetSelected (m_selectionTool == BRUSH,
		 m_beginBinSelection, m_beginBinSelection + 1);
}

void Histogram::SelectionPointMoved (const QPoint &pos)
{
    double value = invTransform(QwtPlot::xBottom, pos.x());
    size_t begin = m_beginBinSelection;
    size_t end = getBin (value);
    if (begin > end)
	swap (begin, end);
    SetSelected (m_selectionTool == BRUSH, begin, end + 1);
}

void Histogram::PolygonSelected (const QwtPolygon& poly)
{
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

void Histogram::EnableSelection (bool enable)
{
    m_plotPicker.setEnabled (enable);
}
