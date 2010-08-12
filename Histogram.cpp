/**
 * @file   Histogram.cpp
 * @author Dan R. Lipsa
 * @date 22 July 2010
 *
 * Definition of the Histogram class
 */

#include "Histogram.h"
#include "DebugStream.h"

Histogram::Histogram (QWidget* parent) :
    QwtPlot (parent), 
    m_plotPicker (QwtPlot::xBottom, QwtPlot::yLeft,
		  QwtPicker::RectSelection | QwtPicker::DragSelection, 
		  QwtPlotPicker::RectRubberBand,
		  QwtPicker::ActiveOnly,
		  canvas())
{
    setCanvasBackground(QColor(Qt::white));

    m_grid.enableXMin(true);
    m_grid.enableYMin(true);
    m_grid.setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    m_grid.setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    m_grid.attach(this);

    m_histogramItem.setColor(Qt::darkCyan);
    m_histogramItem.attach(this);

    m_plotPicker.setRubberBandPen(QColor(Qt::green));

    connect(&m_plotPicker, SIGNAL(moved(const QPoint&)),
	    this, SLOT(Moved(const QPoint&)));

    connect(&m_plotPicker, SIGNAL(appended(const QPoint&)),
	    this, SLOT(Appended(const QPoint&)));

    connect(&m_plotPicker, SIGNAL(selected(const QwtPolygon&)),
	    this, SLOT(Selected(const QwtPolygon&)));
}


void Histogram::Moved (const QPoint &pos)
{
    QString info;
    info.sprintf("Moved: x=%g, y=%g",
		 invTransform(QwtPlot::xBottom, pos.x()),
		 invTransform(QwtPlot::yLeft, pos.y()));
    qDebug () << info;
}

void Histogram::Appended (const QPoint &pos)
{
    QString info;
    info.sprintf("Appended: x=%g, y=%g",
		 invTransform(QwtPlot::xBottom, pos.x()),
		 invTransform(QwtPlot::yLeft, pos.y()));
    qDebug () << info;
}

void Histogram::Selected (const QwtPolygon& poly)
{
    qDebug () << "Selected";
    BOOST_FOREACH (const QPoint& pos, poly)
    {
	QString info;
	info.sprintf("x=%g, y=%g",
		     invTransform(QwtPlot::xBottom, pos.x()),
		     invTransform(QwtPlot::yLeft, pos.y()));
	qDebug () << info;
    }
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

