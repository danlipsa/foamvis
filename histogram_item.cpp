#include "histogram_item.h"

class binToInterval
{
public:
    binToInterval (const QwtIntervalData& intervalData)
	: m_intervalData (intervalData)
    {
    }

    QwtDoubleInterval operator () (const pair<size_t, size_t>& binRange)
    {
	return QwtDoubleInterval (
	    m_intervalData.interval (binRange.first).minValue (),
	    m_intervalData.interval (binRange.second - 1).maxValue ());
    }

private:
    const QwtIntervalData& m_intervalData;
};

const double HistogramItem::logScaleZero = 0.9;

class HistogramItem::PrivateData
{
public:
    PrivateData ()
    {
	reference = 0.0;
	attributes = HistogramItem::Auto;
	logValueAxis = false;
	colorCoded = false;
    }
    int attributes;
    QwtIntervalData data;
    double maxValueAxis;
    QColor focusColor;
    QColor contextColor;
    QColor outOfBoundsColor;
    double reference;
    QBitArray selected;
    bool logValueAxis;
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
    double maxValue,
    const vector< pair<size_t, size_t> >* selectedBins)
{
    d_data->data = data;
    d_data->maxValueAxis = maxValue;
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

void HistogramItem::setMaxValueAxis (double maxValue)
{
    d_data->maxValueAxis = maxValue;
}

double HistogramItem::getMaxValueAxis () const
{
    return d_data->maxValueAxis;
}

void HistogramItem::setAllItemsSelection (bool selected)
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
    y0 = yMap.transform (getMinValueAxis ());
    if (value > d_data->maxValueAxis)
    {
	y2 = yMap.transform (d_data->maxValueAxis);
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



void HistogramItem::draw (QPainter *painter, const QwtScaleMap &xMap, 
			  const QwtScaleMap &yMap, const QRect&) const
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
    const int y1 = yMap.transform(getMinValueAxis ());
    const int y2 = yMap.transform(getMaxValueAxis ());
    int x1 = xMap.transform(iData.interval(beginRegion).minValue());
    int x2 = xMap.transform(iData.interval(endRegion - 1).maxValue());
    QRect paintRect (x1, y1, x2 - x1, y2 - y1);
    QwtPainter::drawRect (painter, paintRect);
}


void HistogramItem::drawBar(
    QPainter *painter, const QColor& color, 
    const QRect& rect, bool outOfBounds) const
{
    const size_t outOfBoundsTop = 5;
    const QRect r = rect.normalized();
    painter->setBrush(color);    
    painter->setPen(Qt::NoPen);
    QwtPainter::drawRect(painter, r.x(), r.y(), r.width(), r.height());
    if (outOfBounds)
    {
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

double HistogramItem::getMinValueAxis () const
{
    return (d_data->logValueAxis ? logScaleZero : 0);
}

bool HistogramItem::isLogValueAxis () const
{
    return d_data->logValueAxis;
}

void HistogramItem::setLogValueAxis (bool logValueAxis)
{
    d_data->logValueAxis = logValueAxis;
}

void HistogramItem::setColorCoded (bool colorCoded)
{
    d_data->colorCoded = colorCoded;
}

void HistogramItem::setColorMap (const QwtLinearColorMap& colorMap)
{
    d_data->colorMap = colorMap;
}
