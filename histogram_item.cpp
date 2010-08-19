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

class HistogramItem::PrivateData
{
public:
    int attributes;
    QwtIntervalData data;
    double maxValue;
    QColor focusColor;
    QColor contextColor;
    double reference;
    QBitArray selected;
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
    d_data->reference = 0.0;
    d_data->attributes = HistogramItem::Auto;

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
    d_data->maxValue = maxValue;
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

void HistogramItem::setSelected (bool selected)
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

    if ( d_data->attributes & Xfy ) 
    {
        rect = QwtDoubleRect( rect.y(), rect.x(), 
            rect.height(), rect.width() );

        if ( rect.left() > d_data->reference ) 
            rect.setLeft( d_data->reference );
        else if ( rect.right() < d_data->reference ) 
            rect.setRight( d_data->reference );
    } 
    else 
    {
        if ( rect.bottom() < d_data->reference ) 
            rect.setBottom( d_data->reference );
        else if ( rect.top() > d_data->reference ) 
            rect.setTop( d_data->reference );
    }

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

void HistogramItem::drawYx (
    size_t i, QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap) const
{
    const QwtIntervalData &iData = d_data->data;
    const int x0 = xMap.transform(baseline());

    const int x2 = xMap.transform(iData.value(i));
    if ( x2 == x0 )
	return;

    int y1 = yMap.transform( iData.interval(i).minValue());
    int y2 = yMap.transform( iData.interval(i).maxValue());
    if ( y1 > y2 )
	qSwap(y1, y2);

    if ( i < iData.size() - 2 )
    {
	const int yy1 = yMap.transform(iData.interval(i+1).minValue());
	const int yy2 = yMap.transform(iData.interval(i+1).maxValue());

	if ( y2 == qwtMin(yy1, yy2) )
	{
	    const int xx2 = xMap.transform(
		iData.interval(i+1).minValue());
	    if ( xx2 != x0 && ( (xx2 < x0 && x2 < x0) ||
				(xx2 > x0 && x2 > x0) ) )
	    {
		// One pixel distance between neighboured bars
		y2++;
	    }
	}
    }

    drawBar(painter, Qt::Horizontal,
	    QRect(x0, y1, x2 - x0, y2 - y1));    
}

void HistogramItem::drawXy (
    size_t i, QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap) const
{
    const QwtIntervalData &iData = d_data->data;
    const int y0 = yMap.transform(baseline());

    const int y2 = yMap.transform(iData.value(i));
    if ( y2 == y0 )
	return;

    int x1 = xMap.transform(iData.interval(i).minValue());
    int x2 = xMap.transform(iData.interval(i).maxValue());
    if ( x1 > x2 )
	qSwap(x1, x2);

    if ( i < iData.size() - 2 )
    {
	const int xx1 = xMap.transform(iData.interval(i+1).minValue());
	const int xx2 = xMap.transform(iData.interval(i+1).maxValue());

	if ( x2 == qwtMin(xx1, xx2) )
	{
	    const int yy2 = yMap.transform(iData.value(i+1));
	    if ( yy2 != y0 && ( (yy2 < y0 && y2 < y0) ||
				(yy2 > y0 && y2 > y0) ) )
	    {
		// One pixel distance between neighboured bars
		x2--;
	    }
	}
    }

    drawBar(painter, Qt::Vertical,
	    QRect(x1, y0, x2 - x1, y2 - y0) );    
}

void HistogramItem::drawHistogramBars (
    QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap) const
{
    const QwtIntervalData &iData = d_data->data;
    for ( size_t i = 0; i < iData.size(); i++ )
    {	
	painter->setPen(
	    QPen((d_data->selected.testBit (i)) ?
		 d_data->focusColor : d_data->contextColor));
        if ( d_data->attributes & HistogramItem::Xfy )
	    drawYx (i, painter, xMap, yMap);
        else
	    drawXy (i, painter, xMap, yMap);
    }    
}


void HistogramItem::draw (QPainter *painter, const QwtScaleMap &xMap, 
			  const QwtScaleMap &yMap, const QRect&) const
{
    drawHistogramBars (painter, xMap, yMap);
    drawSelectionRegions (painter, xMap, yMap);
}

void HistogramItem::drawSelectionRegions (
    QPainter *painter, const QwtScaleMap &xMap, 
    const QwtScaleMap &yMap) const
{
    vector< pair<size_t, size_t> > intervals;
    getSelectedBins (&intervals, false);
    pair<size_t, size_t> interval;
    BOOST_FOREACH (interval, intervals)
	drawRegion (interval.first, interval.second, painter, xMap, yMap);
}

void HistogramItem::drawRegion (
    size_t beginRegion, size_t endRegion,
    QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap) const
{
    const int factor = 100;
    QColor color(d_data->contextColor.lighter (factor));
    const int transparency = 100;
    color.setAlpha (transparency);
    painter->setBrush (color);
    painter->setPen(Qt::NoPen);
    
    const QwtIntervalData &iData = d_data->data;
    QwtDoubleRect rect = boundingRect();
    const int y1 = yMap.transform(baseline ());
    const int y2 = yMap.transform(d_data->maxValue);
    int x1 = xMap.transform(iData.interval(beginRegion).minValue());
    int x2 = xMap.transform(iData.interval(endRegion - 1).maxValue());
    QRect paintRect (x1, y1, x2 - x1, y2 - y1);
    QwtPainter::drawRect (painter, paintRect);
}


void HistogramItem::drawBar(QPainter *painter,
   Qt::Orientation, const QRect& rect) const
{
    painter->save();

    const QColor color(painter->pen().color());
    const QRect r = rect.normalized();

    const int factor = 125;
    const QColor light(color.light(factor));
    const QColor dark(color.dark(factor));
    
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    QwtPainter::drawRect(painter, r.x() + 1, r.y() + 1,
			 r.width() - 2, r.height() - 2);

    painter->setBrush(Qt::NoBrush);    
    painter->setPen(QPen(light, 2));
    QwtPainter::drawLine(painter,
			 r.left() + 1, r.top() + 2, r.right() + 1, r.top() + 2);
    
    painter->setPen(QPen(dark, 2));
    QwtPainter::drawLine(painter, 
			 r.left() + 1, r.bottom(), r.right() + 1, r.bottom());
    
    painter->setPen(QPen(light, 1));    
    QwtPainter::drawLine(
	painter, r.left(), r.top() + 1, r.left(), r.bottom());
    QwtPainter::drawLine(
	painter, r.left() + 1, r.top() + 2, r.left() + 1, r.bottom() - 1);
    
 
    painter->setPen(QPen(dark, 1));
    QwtPainter::drawLine(painter, 
			 r.right() + 1, r.top() + 1, r.right() + 1, r.bottom());
    QwtPainter::drawLine(painter, 
			 r.right(), r.top() + 2, r.right(), r.bottom() - 1);
    painter->restore();
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
