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
    m_histogramItem.setOutOfBoundsColor(Qt::red);
    m_histogramItem.attach(this);

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
    m_histogramItem.setSelected (m_selectionTool == BRUSH,
				 m_beginBinSelection, m_beginBinSelection + 1);
}

void Histogram::SelectionPointMoved (const QPoint& pos)
{
    double value = invTransform(QwtPlot::xBottom, pos.x());
    size_t begin = m_beginBinSelection;
    size_t end = getBin (value);
    if (begin > end)
	swap (begin, end);
    m_histogramItem.setSelected (m_selectionTool == BRUSH, begin, end + 1);
}

void Histogram::PolygonSelected (const QwtPolygon& poly)
{
    (void) poly;
    Q_EMIT selectionChanged ();
}

void Histogram::SetSelected (bool selected)
{
    m_histogramItem.setSelected (selected);
    Q_EMIT selectionChanged ();
}


void Histogram::SetData (
    const QwtIntervalData& intervalData, double maxValue,
    const vector< pair<size_t, size_t> >* selectedBins)
{
    m_axisMaxValue = maxValue;
    m_histogramItem.setData(intervalData, maxValue, selectedBins);
    setAxisScale(QwtPlot::yLeft, 0.0, maxValue);
    setAxisScale(
	QwtPlot::xBottom,
	intervalData.interval (0).minValue (),
	intervalData.interval (intervalData.size () - 1).maxValue ());
}

void Histogram::SetAxisMaxValue (double axisMaxValue)
{
    m_axisMaxValue = axisMaxValue;
    m_histogramItem.setMaxValue (axisMaxValue);
    setAxisScale(QwtPlot::yLeft, 0.0, axisMaxValue);
}

void Histogram::EnableSelection (bool enable)
{
    m_plotPicker.setEnabled (enable);
}

void Histogram::GetSelectedIntervals (
    vector<QwtDoubleInterval>* intervals) const
{
    m_histogramItem.getSelectedIntervals (intervals);
}

size_t Histogram::GetDataMaxValue () const
{
    QwtDoubleRect rect = m_histogramItem.boundingRect ();
    return rect.y () + rect.height ();
}
