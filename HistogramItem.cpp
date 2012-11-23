#include "HistogramItem.h"


const double HistogramItem::logScaleZero = 0.9;

class HistogramItem::PrivateData
{
public:
    PrivateData () :
        attributes (HistogramItem::Auto),
	reference (0.0),
        xAxisLogScale (false),
        xAxisMaxValue (0),
        xAxisMinValue (0),
	yAxisLogScale (false),
        yAxisMaxValue (0),
	colorCoded (false)
    {
    }
    
    int attributes;
    QwtIntervalData data;
    QColor focusColor;
    QColor contextColor;
    QColor outOfBoundsColor;
    double reference;
    QBitArray selected;
    bool xAxisLogScale;
    double xAxisMaxValue;
    double xAxisMinValue;
    bool yAxisLogScale;
    double yAxisMaxValue;
    bool colorCoded;
    QwtLinearColorMap colorMap;
};


HistogramItem::HistogramItem(const QwtText &title):
    QwtPlotItem(title)
{
    init();
}

HistogramItem::HistogramItem(const QString &title):
    QwtPlotItem(QwtText(title))
{
    init();
}

HistogramItem::~HistogramItem()
{
    delete d_data;
}

void HistogramItem::init()
{
    d_data = new PrivateData();    
    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, true);
    setZ(20.0);
}

void HistogramItem::setBaseline(double reference)
{
    if ( d_data->reference != reference )
    {
        d_data->reference = reference;
        itemChanged();
    }
}

double HistogramItem::baseline() const
{
    return d_data->reference;
}

void HistogramItem::setData(
    const QwtIntervalData &data,
    double yAxisMaxValue,
    const vector< pair<size_t, size_t> >* selectedBins)
{
    d_data->data = data;
    d_data->yAxisMaxValue = yAxisMaxValue;
    d_data->selected.resize (data.size ());

    if (selectedBins != 0)
    {
	d_data->selected.fill (false);
	setSelectedBins (*selectedBins);
    }
    else
	d_data->selected.fill (true);	
    itemChanged();
}

void HistogramItem::SetYAxisMaxValue (double yAxisMaxValue)
{
    d_data->yAxisMaxValue = yAxisMaxValue;
}

double HistogramItem::GetYAxisMaxValue () const
{
    return d_data->yAxisMaxValue;
}

void HistogramItem::SetXAxisMaxValue (double xAxisMaxValue)
{
    d_data->xAxisMaxValue = xAxisMaxValue;
}

double HistogramItem::GetXAxisMaxValue () const
{
    return d_data->xAxisMaxValue;
}

void HistogramItem::SetXAxisMinValue (double xAxisMinValue)
{
    d_data->xAxisMinValue = xAxisMinValue;
}

double HistogramItem::GetXAxisMinValue () const
{
    return d_data->xAxisMinValue;
}

void HistogramItem::setAllItemsSelected (bool selected)
{
    d_data->selected.fill (selected);
    itemChanged ();
}

void HistogramItem::setSelected (bool selected, size_t begin, size_t end)
{
    d_data->selected.fill (selected, begin, end);
    itemChanged ();
}

const QwtIntervalData &HistogramItem::data() const
{
    return d_data->data;
}

void HistogramItem::setFocusColor(const QColor &color)
{
    if ( d_data->focusColor != color )
    {
        d_data->focusColor = color;
        itemChanged();
    }
}

void HistogramItem::setContextColor(const QColor &color)
{
    if ( d_data->contextColor != color )
    {
        d_data->contextColor = color;
        itemChanged();
    }
}


QColor HistogramItem::focusColor() const
{
    return d_data->focusColor;
}

QColor HistogramItem::contextColor() const
{
    return d_data->contextColor;
}

QwtDoubleRect HistogramItem::boundingRect() const
{
    QwtDoubleRect rect = d_data->data.boundingRect();
    if ( !rect.isValid() ) 
        return rect;
    if ( rect.bottom() < d_data->reference ) 
	rect.setBottom( d_data->reference );
    else if ( rect.top() > d_data->reference ) 
	rect.setTop( d_data->reference );

    return rect;
}


int HistogramItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotHistogram;
}

void HistogramItem::setHistogramAttribute(HistogramAttribute attribute, bool on)
{
    if ( bool(d_data->attributes & attribute) == on )
        return;

    if ( on )
        d_data->attributes |= attribute;
    else
        d_data->attributes &= ~attribute;

    itemChanged();
}

bool HistogramItem::testHistogramAttribute(HistogramAttribute attribute) const
{
    return d_data->attributes & attribute;
}

void HistogramItem::drawBars (
    QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap) const
{
    const QwtIntervalData &iData = d_data->data;
    for ( size_t i = 0; i < iData.size(); i++ )
	drawBar (i, painter, xMap, yMap);
}

void HistogramItem::drawBar (
    size_t i, QPainter *painter, 
    const QwtScaleMap &xMap, const QwtScaleMap &yMap) const
{
    bool outside = false;
    const QwtIntervalData &iData = d_data->data;
    int y0 = yMap.transform(baseline ());
    
    double value = iData.value (i);

    int y2 = yMap.transform(value);
    if ( y2 == y0 )
	return;
    y0 = yMap.transform (GetYAxisMinValue ());
    if (value > d_data->yAxisMaxValue)
    {
	y2 = yMap.transform (d_data->yAxisMaxValue);
	outside = true;
    }

    int x1 = xMap.transform(iData.interval(i).minValue());
    int x2 = xMap.transform(iData.interval(i).maxValue());
    if ( x1 > x2 )
	qSwap(x1, x2);

    drawBar(painter, getBarColor (i), 
	    QRect(x1, y0, x2 - x1, y2 - y0), outside);
}

QColor HistogramItem::getBarColor (size_t i) const
{
    if (d_data->colorCoded)
    {
	if (d_data->selected.testBit (i))
	{
	    const QwtIntervalData &iData = d_data->data;
	    size_t bars = iData.size ();
	    double value = static_cast<double> (i) / (bars - 1);
	    return d_data->colorMap.color (QwtDoubleInterval (0, 1), value);
	}
	else
	    return d_data->contextColor;
    }
    else
	return (d_data->selected.testBit (i)) ? 
	    d_data->focusColor : 
	    d_data->contextColor;
}



void HistogramItem::draw (QPainter *painter,
			  const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
			  const QRect&) const
{
    drawBars (painter, xMap, yMap);
    drawDeselectedRegions (painter, xMap, yMap);
}

void HistogramItem::drawDeselectedRegions (
    QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap) const
{
    vector< pair<size_t, size_t> > intervals;
    getSelectedBins (&intervals, false);
    pair<size_t, size_t> interval;
    BOOST_FOREACH (interval, intervals)
	drawDeselectedRegion (
	    interval.first, interval.second, painter, xMap, yMap);
}

void HistogramItem::drawDeselectedRegion (
    size_t beginRegion, size_t endRegion,
    QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap) const
{
    const int factor = 100;
    const int transparency = 100;
    QColor color(d_data->contextColor.lighter (factor));
    color.setAlpha (transparency);
    painter->setBrush (color);
    painter->setPen(Qt::NoPen);

    const QwtIntervalData &iData = d_data->data;
    const int y1 = yMap.transform(GetYAxisMinValue ());
    const int y2 = yMap.transform(GetYAxisMaxValue ());
    int x1 = xMap.transform(iData.interval(beginRegion).minValue());
    int x2 = xMap.transform(iData.interval(endRegion - 1).maxValue());
    QRect paintRect (x1, y1, x2 - x1, y2 - y1);
    QwtPainter::drawRect (painter, paintRect);
}


void HistogramItem::drawBar(
    QPainter *painter, const QColor& color, 
    const QRect& rect, bool outOfBounds) const
{
    const QRect r = rect.normalized();
    painter->setBrush(color);    
    painter->setPen(Qt::NoPen);
    QwtPainter::drawRect(painter, r.x(), r.y(), r.width(), r.height());
    if (outOfBounds)
    {
        const size_t outOfBoundsTop = 5;
	painter->setBrush(d_data->outOfBoundsColor);    
	painter->setPen(QPen(Qt::black));
	QwtPainter::drawRect(painter, r.x(), r.y(),
			     r.width(), outOfBoundsTop);
    }
}

void HistogramItem::getSelectedBins (
    vector< pair<size_t, size_t> >* bins, bool selected) const
{
    const QwtIntervalData &iData = d_data->data;
    size_t beginRegion = 0;
    bool regionSelection = d_data->selected.testBit (0);
    for ( size_t i = 1; i < iData.size(); i++ )
    {
	bool currentSelection = d_data->selected.testBit (i);
	if ( currentSelection != regionSelection)
	{
	    if (regionSelection == selected)
		bins->push_back (pair<size_t, size_t> (beginRegion, i));
	    beginRegion = i;
	    regionSelection = currentSelection;
	}
    }
    if (regionSelection == selected)
	bins->push_back (
	    pair<size_t, size_t> (beginRegion, iData.size ()));
}

class binToInterval
{
public:
    binToInterval (const QwtIntervalData& intervalData)
	: m_intervalData (intervalData)
    {
    }

    QwtDoubleInterval operator () (const pair<size_t, size_t>& binRange)
    {
	QwtDoubleInterval interval (
	    m_intervalData.interval (binRange.first).minValue (),
	    m_intervalData.interval (binRange.second - 1).maxValue (),
	    (m_intervalData.size () == binRange.second) ?
	    QwtDoubleInterval::IncludeBorders : 
	    QwtDoubleInterval::ExcludeMaximum);
	return interval;
    }

private:
    const QwtIntervalData& m_intervalData;
};
void HistogramItem::getSelectedIntervals (
    vector<QwtDoubleInterval>* intervals) const
{
    const QwtIntervalData &iData = d_data->data;
    vector< pair<size_t, size_t> > bins;
    getSelectedBins (&bins);
    intervals->resize (bins.size ());
    std::transform (bins.begin (), bins.end (), intervals->begin (),
		    binToInterval (iData));
}



void HistogramItem::setSelectedBins (
    const vector< pair<size_t, size_t> >& intervals)
{
    pair<size_t, size_t> interval;
    BOOST_FOREACH (interval, intervals)
	setSelected (true, interval.first, interval.second);
}

void HistogramItem::setOutOfBoundsColor (const QColor& color)
{
    d_data->outOfBoundsColor = color;
}

double HistogramItem::GetYAxisMinValue () const
{
    return (d_data->yAxisLogScale ? logScaleZero : 0);
}

bool HistogramItem::IsYAxisLogScale () const
{
    return d_data->yAxisLogScale;
}

void HistogramItem::SetYAxisLogScale (bool logYAxis)
{
    d_data->yAxisLogScale = logYAxis;
}

bool HistogramItem::IsXAxisLogScale () const
{
    return d_data->xAxisLogScale;
}

void HistogramItem::SetXAxisLogScale (bool logAxis)
{
    d_data->xAxisLogScale = logAxis;
}


void HistogramItem::setColorCoded (bool colorCoded)
{
    d_data->colorCoded = colorCoded;
}

void HistogramItem::setColorMap (const QwtLinearColorMap& colorMap)
{
    d_data->colorMap = colorMap;
}

const QwtLinearColorMap& HistogramItem::getColorMap () const
{
    return d_data->colorMap;
}


const QwtDoubleInterval HistogramItem::GetDataInterval () const
{
    const QwtIntervalData data = d_data->data;
    return QwtDoubleInterval (
	data.interval(0).minValue (), 
	data.interval (data.size () - 1).maxValue ());
}
