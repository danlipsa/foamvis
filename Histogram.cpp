/**
 * @file   Histogram.cpp
 * @author Dan R. Lipsa
 * @date 22 July 2010
 *
 * Definition of the Histogram class
 */

#include "Application.h"
#include "DebugStream.h"
#include "Histogram.h"
#include "HistogramHeight.h"
#include "Statistics.h"


const QSize Histogram::SIZE_HINT (200, 200);

/**
 * @todo Add an option to show percentage per bin instead of count per
 * bin for the y axis of the histogram.
 */
Histogram::Histogram (QWidget* parent) :
    QwtPlot (parent), 
    m_plotPicker (QwtPlot::xBottom, QwtPlot::yLeft,
		  QwtPicker::RectSelection | QwtPicker::DragSelection, 
		  QwtPlotPicker::NoRubberBand,
		  QwtPicker::AlwaysOff,
		  canvas()),
    m_selectionTool (ERASER),
    m_histogramHeight (new HistogramHeight (this))
{
    setCanvasBackground(QColor(Qt::white));
    alignScales ();
    setAutoReplot ();
    setAxisTitleDefaultFont (QwtPlot::yLeft, "Count per bin");
    setAxisDefaultFont (QwtPlot::yLeft);
    setAxisDefaultFont (QwtPlot::xBottom);

    m_grid.setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    m_grid.setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    m_grid.attach(this);
    SetGridEnabled (true);

    m_histogramItem.setFocusColor(Qt::darkCyan);
    m_histogramItem.setContextColor(QColor(Qt::lightGray).lighter (110));
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

void Histogram::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    canvas()->setFrameStyle(QFrame::Box | QFrame::Plain );
    canvas()->setLineWidth(1);

    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(i);
        if ( scaleWidget )
            scaleWidget->setMargin(0);

        QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axisScaleDraw(i);
        if ( scaleDraw )
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }
}



size_t Histogram::getBin (double value)
{
    const QwtIntervalData& data = m_histogramItem.data ();
    size_t binCount = data.size ();
    return HistogramStatistics::GetBin (
	value, binCount,
	data.interval (0).minValue (), data.interval (binCount - 1).maxValue ());
}

void Histogram::SelectionPointAppended (const QPoint &canvasPos)
{
    double value = invTransform(QwtPlot::xBottom, canvasPos.x());
    m_beginBinSelection = getBin (value);
    m_histogramItem.setSelected (m_selectionTool == BRUSH,
				 m_beginBinSelection, m_beginBinSelection + 1);
}

void Histogram::SelectionPointMoved (const QPoint& canvasPos)
{
    double value = invTransform(QwtPlot::xBottom, canvasPos.x());
    size_t begin = m_beginBinSelection;
    size_t end = getBin (value);
    if (begin > end)
	swap (begin, end);
    m_histogramItem.setSelected (m_selectionTool == BRUSH, begin, end + 1);
}

void Histogram::PolygonSelected (const QwtPolygon& poly)
{
    static_cast<void> (poly);
    Q_EMIT selectionChanged ();
}

void Histogram::SetAllItemsSelection (bool selected)
{
    m_histogramItem.setAllItemsSelection (selected);
    Q_EMIT selectionChanged ();
}

bool Histogram::AreAllItemsSelected () const
{
    vector< pair<size_t, size_t> > selectedBins;
    GetSelectedBins (&selectedBins);
    return selectedBins.size () == 1 &&
	selectedBins[0].first == 0 && 
	selectedBins[0].second == m_histogramItem.data ().size ();
}

void Histogram::SetSelectionTool (SelectionTool selectionTool)
{
    m_selectionTool = selectionTool;    
    m_plotPicker.setEnabled (selectionTool == NONE ? false : true);
}

void Histogram::setAxisTitleDefaultFont (int axisId, const char* s)
{
    QFont defaultFont = Application::Get ()->font ();
    defaultFont.setBold (true);
    QwtText at;
    if (s != 0)
	at.setText (s);
    else
	at = axisTitle (axisId);
    at.setFont (defaultFont);
    setAxisTitle (axisId, at);
}

void Histogram::setAxisDefaultFont (int axisId)
{
    QFont defaultFont = Application::Get ()->font ();
    setAxisFont (axisId, defaultFont);
}

void Histogram::SetDefaultFont ()
{
    setAxisTitleDefaultFont (QwtPlot::xBottom);
    setAxisTitleDefaultFont (QwtPlot::yLeft);
    setAxisDefaultFont (QwtPlot::xBottom);
    setAxisDefaultFont (QwtPlot::yLeft);
}


void Histogram::SetDataAllBinsSelected (
    const QwtIntervalData& intervalData, double maxValue, const char* axisTitle)
{
    setData (intervalData, maxValue);
    setAxisTitleDefaultFont (QwtPlot::xBottom, axisTitle);
    replot ();
    Q_EMIT selectionChanged ();
}


void Histogram::SetDataKeepBinSelection (
    const QwtIntervalData& intervalData, double maxValue, const char* axisTitle)
{
    if (m_histogramItem.data ().size () == 0)
	SetDataAllBinsSelected (intervalData, maxValue, axisTitle);
    else
    {
	vector< pair<size_t, size_t> > selectedBins;
	GetSelectedBins (&selectedBins);
	setData (intervalData, maxValue, &selectedBins);
	setAxisTitleDefaultFont (QwtPlot::xBottom, axisTitle);
	replot ();
    }
}


void Histogram::setData (
    const QwtIntervalData& intervalData, double maxValue,
    const vector< pair<size_t, size_t> >* selectedBins)
{
    m_histogramItem.setData(intervalData, maxValue, selectedBins);
    setAxisScale(QwtPlot::yLeft, GetMinValueAxis (), maxValue);
    setAxisScale(
	QwtPlot::xBottom,
	intervalData.interval (0).minValue (),
	intervalData.interval (intervalData.size () - 1).maxValue ());
}

void Histogram::SetMaxValueAxis (double maxValueAxis)
{
    m_histogramItem.setMaxValueAxis (maxValueAxis);
    setAxisScale(QwtPlot::yLeft, GetMinValueAxis (), maxValueAxis);
}

void Histogram::SetGridEnabled (bool enabled)
{
    m_grid.enableX (enabled);
    m_grid.enableY (enabled);
    m_grid.enableXMin (enabled);
    m_grid.enableYMin (enabled);
}

bool Histogram::IsGridEnabled () const
{
    return m_grid.xEnabled ();
}

void Histogram::GetSelectedIntervals (
    vector<QwtDoubleInterval>* intervals) const
{
    m_histogramItem.getSelectedIntervals (intervals);
}

size_t Histogram::GetMaxValueData () const
{
    QwtDoubleRect rect = m_histogramItem.boundingRect ();
    return rect.y () + rect.height ();
}

void Histogram::SetLogValueAxis (bool logValueAxis)
{
    m_histogramItem.setLogValueAxis (logValueAxis);
    SetMaxValueAxis (GetMaxValueAxis ());
    if (logValueAxis)
	setAxisScaleEngine (QwtPlot::yLeft, new QwtLog10ScaleEngine);
    else
	setAxisScaleEngine (QwtPlot::yLeft, new QwtLinearScaleEngine);
}

void Histogram::SetDisplayColorBar (bool displayColorBar)
{    
    m_displayColorBar = displayColorBar;
    QwtScaleWidget* scaleWidget = axisWidget (QwtPlot::xBottom);
    scaleWidget->setColorBarEnabled (displayColorBar);
}

void Histogram::SetColorMap (const QwtDoubleInterval& interval, 
			     const QwtLinearColorMap& colorMap)
{
    m_histogramItem.setColorMap (colorMap);    
    QwtScaleWidget* scaleWidget = axisWidget (QwtPlot::xBottom);
    scaleWidget->setColorMap (interval, m_histogramItem.getColorMap ());
}

void Histogram::HistogramHeightDialog ()
{
    m_histogramHeight->SetValue (GetMaxValueAxis ());
    m_histogramHeight->SetMaximumValue (GetMaxValueData ());
    m_histogramHeight->SetLogScale (IsLogValueAxis ());
    if (m_histogramHeight->exec () == QDialog::Accepted)
    {
	SetLogValueAxis (
	    m_histogramHeight->IsLogScale () ? true : false);
	SetMaxValueAxis (m_histogramHeight->GetValue ());
    }
}

void Histogram::SetItemsSelectionHigh (bool selected, double value)
{
    size_t begin = getBin (value);
    size_t binCount = m_histogramItem.data ().size ();
    m_histogramItem.setSelected (selected, begin, binCount);
}

void Histogram::SetItemsSelectionLow (bool selected, double value)
{
    size_t end = getBin (value);
    m_histogramItem.setSelected (selected, 0, end + 1);
}

QSize Histogram::sizeHint () const
{
    return SIZE_HINT;
}
