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
#include "HistogramSettings.h"
#include "HistogramStatistics.h"
#include "HistogramItem.h"


const QSize Histogram::SIZE_HINT (200, 200);

/**
 * @todo Add an option to show percentage per bin instead of count per
 * bin for the y axis of the histogram.
 */
Histogram::Histogram (QWidget* parent) :
    QwtPlot (parent), 
    m_histogramItem (new HistogramItem ()),
    m_plotPicker (QwtPlot::xBottom, QwtPlot::yLeft,
		  QwtPicker::RectSelection | QwtPicker::DragSelection, 
		  QwtPlotPicker::NoRubberBand,
		  QwtPicker::AlwaysOff,
		  canvas()),
    m_selectionTool (ERASER),
    m_histogramHeight (new HistogramSettings (this)),
    m_sizeHint (SIZE_HINT)
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

    m_histogramItem->setFocusColor(Qt::darkCyan);
    m_histogramItem->setContextColor(QColor(Qt::lightGray).lighter (110));
    m_histogramItem->setOutOfBoundsColor(Qt::red);
    m_histogramItem->attach(this);

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
    const QwtIntervalData& data = m_histogramItem->data ();
    size_t binCount = data.size ();
    return HistogramStatistics::GetBin (
	value, binCount,
	data.interval (0).minValue (), data.interval (binCount - 1).maxValue ());
}

void Histogram::SelectionPointAppended (const QPoint &canvasPos)
{
    double value = invTransform(QwtPlot::xBottom, canvasPos.x());
    m_beginBinSelection = getBin (value);
    m_histogramItem->setSelected (m_selectionTool == BRUSH,
				 m_beginBinSelection, m_beginBinSelection + 1);
}

void Histogram::SelectionPointMoved (const QPoint& canvasPos)
{
    double value = invTransform(QwtPlot::xBottom, canvasPos.x());
    size_t begin = m_beginBinSelection;
    size_t end = getBin (value);
    if (begin > end)
	swap (begin, end);
    m_histogramItem->setSelected (m_selectionTool == BRUSH, begin, end + 1);
}

void Histogram::PolygonSelected (const QwtPolygon& poly)
{
    static_cast<void> (poly);
    Q_EMIT SelectionChanged ();
}

void Histogram::SetAllItemsSelection (bool selected)
{
    m_histogramItem->setAllItemsSelected (selected);
    Q_EMIT SelectionChanged ();
}

bool Histogram::AreAllItemsSelected () const
{
    BinRegions selectedBins;
    GetSelectedBins (&selectedBins);
    return selectedBins.size () == 1 &&
	selectedBins[0].first == 0 && 
	selectedBins[0].second == m_histogramItem->data ().size ();
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
    Q_EMIT SelectionChanged ();
}

void Histogram::SetSelectedBinsNoSignal (const BinRegions& bins)
{
    m_histogramItem->setAllItemsSelected (false);
    m_histogramItem->setSelectedBins (bins);
    replot ();
}


void Histogram::SetDataKeepBinSelection (
    const QwtIntervalData& intervalData, double maxValue, 
    const char* axisTitle, const BinRegions& selectedBins)
{
    setData (intervalData, maxValue, &selectedBins);
    setAxisTitleDefaultFont (QwtPlot::xBottom, axisTitle);
    replot ();
}


bool Histogram::HasData () const
{
    return m_histogramItem->HasData ();
}

void Histogram::setData (
    const QwtIntervalData& intervalData, double maxValue,
    const BinRegions* selectedBins)
{
    m_histogramItem->setData(intervalData, maxValue, selectedBins);
    setAxisScale(QwtPlot::yLeft, GetYAxisMinValue (), maxValue);

    SetXAxisMinValue (intervalData.interval (0).minValue ());
    SetXAxisMaxValue (
        intervalData.interval (intervalData.size () - 1).maxValue ());
    setAxisScale(QwtPlot::xBottom, GetXAxisMinValue (), GetXAxisMaxValue ());
}

void Histogram::SetYAxisMaxValue (double maxValueAxis)
{
    m_histogramItem->SetYAxisMaxValue (maxValueAxis);
    setAxisScale(QwtPlot::yLeft, GetYAxisMinValue (), maxValueAxis);
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
    m_histogramItem->getSelectedIntervals (intervals);
}

size_t Histogram::GetYAxisMaxValueData () const
{
    QwtDoubleRect rect = m_histogramItem->boundingRect ();
    return rect.y () + rect.height ();
}

void Histogram::SetYAxisLogScale (bool logYAxis)
{
    m_histogramItem->SetYAxisLogScale (logYAxis);
    SetYAxisMaxValue (GetYAxisMaxValue ());
    if (logYAxis)
	setAxisScaleEngine (QwtPlot::yLeft, new QwtLog10ScaleEngine);
    else
	setAxisScaleEngine (QwtPlot::yLeft, new QwtLinearScaleEngine);
}

void Histogram::SetXAxisLogScale (bool logXAxis)
{
    m_histogramItem->SetXAxisLogScale (logXAxis);
    if (logXAxis)
	setAxisScaleEngine (QwtPlot::xBottom, new QwtLog10ScaleEngine);
    else
	setAxisScaleEngine (QwtPlot::xBottom, new QwtLinearScaleEngine);
}


void Histogram::SetDisplayColorBar (bool displayColorBar)
{    
    m_displayColorBar = displayColorBar;
    QwtScaleWidget* scaleWidget = axisWidget (QwtPlot::xBottom);
    scaleWidget->setColorBarEnabled (displayColorBar);
}

void Histogram::SetColorTransferFunction (const QwtDoubleInterval& interval, 
					  const QwtLinearColorMap& colorMap)
{
    m_histogramItem->setColorMap (colorMap);    
    QwtScaleWidget* scaleWidget = axisWidget (QwtPlot::xBottom);
    scaleWidget->setColorMap (interval, m_histogramItem->getColorMap ());
}

void Histogram::HistogramSettingsDialog ()
{
    m_histogramHeight->SetYValue (GetYAxisMaxValue ());
    m_histogramHeight->SetYAxisMaxValue (GetYAxisMaxValueData ());
    m_histogramHeight->SetYAxisLogScale (IsYAxisLogScale ());
    if (m_histogramHeight->exec () == QDialog::Accepted)
    {
	SetYAxisLogScale (m_histogramHeight->IsYAxisLogScale ());
	SetYAxisMaxValue (m_histogramHeight->GetYValue ());
    }
}

void Histogram::SetItemsSelectionHigh (bool selected, double value)
{
    size_t begin = getBin (value);
    size_t binCount = m_histogramItem->data ().size ();
    m_histogramItem->setSelected (selected, begin, binCount);
}

void Histogram::SetItemsSelectionLow (bool selected, double value)
{
    size_t end = getBin (value);
    m_histogramItem->setSelected (selected, 0, end + 1);
}

QSize Histogram::sizeHint () const
{
    return m_sizeHint;
}

void Histogram::SetColorCoded (bool colorCoded)
{
    m_histogramItem->setColorCoded (colorCoded);
}

double Histogram::GetYAxisMaxValue () const
{
    return m_histogramItem->GetYAxisMaxValue ();
}

double Histogram::GetYAxisMinValue () const
{
    return m_histogramItem->GetYAxisMinValue ();
}

double Histogram::GetXAxisMaxValue () const
{
    return m_histogramItem->GetXAxisMaxValue ();
}

void Histogram::SetXAxisMaxValue (double value)
{
    m_histogramItem->SetXAxisMaxValue (value);
}


double Histogram::GetXAxisMinValue () const
{
    return m_histogramItem->GetXAxisMinValue ();
}

void Histogram::SetXAxisMinValue (double value)
{
    m_histogramItem->SetXAxisMinValue (value);
}

bool Histogram::IsYAxisLogScale () const
{
    return m_histogramItem->IsYAxisLogScale ();
}

void Histogram::GetSelectedBins (BinRegions* intervals, bool selected) const
{
    m_histogramItem->getSelectedBins (intervals, selected);
}
