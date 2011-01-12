/**
 * @file   BodySetStatistics.cpp
 * @author Dan R. Lipsa
 * @date 8 August 2010
 *
 * Implementation of the BodySetStatistics class
 */

#include "Body.h"
#include "BodySetStatistics.h"
#include "FoamAlongTime.h"
#include "Utils.h"


BodySetStatistics::BodySetStatistics () :
    m_min (BodyProperty::PROPERTY_END, numeric_limits<double> ().max ()),
    m_max (BodyProperty::PROPERTY_END, -numeric_limits<double> ().max ()),
    m_median (BodyProperty::PROPERTY_END, 0),
    m_histogram (BodyProperty::PROPERTY_END),
    m_maxCountPerBin (BodyProperty::PROPERTY_END),
    m_totalCount (BodyProperty::PROPERTY_END, 0)
{
    BOOST_FOREACH (vector<size_t>& v, m_histogram)
	v.resize (HISTOGRAM_INTERVALS, 0);
}

void BodySetStatistics::Initialize ()
{
    fill (m_min.begin (), m_min.end (), numeric_limits<double> ().max ());
    fill (m_max.begin (), m_max.end (), -numeric_limits<double> ().max ());
    fill (m_totalCount.begin (), m_totalCount.end (), 0);
}


void BodySetStatistics::SpeedHistogramStep (
    const StripIterator::Point& beforeBegin,
    const StripIterator::Point& begin,
    const StripIterator::Point& end,
    const StripIterator::Point& afterEnd)
{
    static_cast<void>(beforeBegin);
    static_cast<void>(afterEnd);
    G3D::Vector3 speed = end.m_point - begin.m_point;
    boost::array<double, 4> speedComponents = 
	{{speed.x, speed.y, speed.z, speed.length ()}};
    for (size_t i = BodyProperty::VELOCITY_BEGIN;
	 i < BodyProperty::VELOCITY_END; ++i)
	valuePerInterval (i, speedComponents[i], GetMin (i), GetMax (i));
}

void BodySetStatistics::HistogramStep (const boost::shared_ptr<Body>& body)
{
    for (size_t i = BodyProperty::PER_BODY_BEGIN;
	 i < BodyProperty::PER_BODY_END; ++i)
    {
	size_t index = i - BodyProperty::PER_BODY_BEGIN;
	if (body->ExistsAttribute (index))
	    valuePerInterval (i, body->GetRealAttribute (index), 
			      GetMin (i), GetMax (i));
    }
}

void BodySetStatistics::HistogramStep (
    const FoamAlongTime& foamAlongTime, 
    size_t bodyId, size_t timeStep, const BodySetStatistics& rangeStatistics)
{
    for (size_t i = BodyProperty::PROPERTY_BEGIN; 
	 i < BodyProperty::PROPERTY_END; ++i)
    {
	BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT(i);
	if (foamAlongTime.ExistsBodyProperty (bodyProperty, bodyId, timeStep))
	    valuePerInterval (
		i, foamAlongTime.GetBodyPropertyValue (
		    bodyProperty, bodyId, timeStep),
		rangeStatistics.GetMin (i), rangeStatistics.GetMax (i));
    }
}


void BodySetStatistics::valuePerInterval (
    size_t i, double value, 
    double beginInterval, double endInterval)
{
    size_t bin = GetBin (
	value, HistogramIntervals (), beginInterval, endInterval);
    ++m_histogram[i][bin];
}

size_t BodySetStatistics::GetBin (
    double value, size_t binCount, double beginInterval, double endInterval)
{
    if (beginInterval == endInterval || value < beginInterval)
	return 0;
    else if (value >= endInterval)
	return binCount - 1;
    else
    {
	double step = (endInterval - beginInterval) / binCount;
	return floor ((value - beginInterval) / step);
    }
}


void BodySetStatistics::RangeStep (size_t bodyProperty, double newValue)
{
    MinStep (bodyProperty, newValue);
    MaxStep (bodyProperty, newValue);
    ++m_totalCount [bodyProperty];
}

void BodySetStatistics::normalizeEmptyRange (size_t bodyProperty)
{
    if (GetMin (bodyProperty) > GetMax (bodyProperty))
	m_min[bodyProperty] = m_max[bodyProperty] = 0;
}

void BodySetStatistics::NormalizeEmptyRange ()
{
    for (size_t i = BodyProperty::PROPERTY_BEGIN; 
	 i < BodyProperty::PROPERTY_END; ++i)
	normalizeEmptyRange (i);
}

QwtIntervalData BodySetStatistics::GetHistogram (
    size_t bodyProperty, const BodySetStatistics* rangeStatistics) const
{
    if (rangeStatistics == 0)
	rangeStatistics = this;
    size_t histogramIntervals = HistogramIntervals ();
    QwtArray<QwtDoubleInterval> intervals (histogramIntervals);
    QwtArray<double> values (histogramIntervals);
    double beginInterval = rangeStatistics->GetMin (bodyProperty);
    double endInterval = rangeStatistics->GetMax (bodyProperty);
    double step = (endInterval - beginInterval) / histogramIntervals;
    double pos = beginInterval;
    for (size_t bin = 0; bin < histogramIntervals; ++bin)
    {
	intervals[bin] = QwtDoubleInterval (
	    pos, pos + step, 
	    bin == (histogramIntervals - 1) ? 
	    QwtDoubleInterval::IncludeBorders : 
	    QwtDoubleInterval::ExcludeMaximum);
	values[bin] = GetValuesPerBin (bodyProperty, bin);
	pos += step;
    }
    return QwtIntervalData (intervals, values);
}

void BodySetStatistics::CalculateMaxCountPerBin ()
{
    for (size_t bodyProperty = BodyProperty::PROPERTY_BEGIN;
	 bodyProperty < BodyProperty::PROPERTY_END; ++bodyProperty)
    {
	size_t maxCount = 0;
	size_t histogramIntervals = HistogramIntervals ();
	for (size_t bin = 0; bin < histogramIntervals; ++bin)
	    maxCount = max (maxCount, GetValuesPerBin (bodyProperty, bin));
	m_maxCountPerBin[bodyProperty] = maxCount;
    }
}

void BodySetStatistics::ApproximateMedian ()
{
    for (size_t bodyProperty = BodyProperty::PROPERTY_BEGIN;
	 bodyProperty < BodyProperty::PROPERTY_END; ++bodyProperty)
    {
	size_t histogramIntervals = HistogramIntervals ();
	size_t countSoFar = 0;
	size_t currentCount;
	double limit = GetTotalCount (
	    BodyProperty::FromSizeT (bodyProperty)) / 2.0;
	size_t bin = 0;
	while (bin < histogramIntervals)
	{
	    currentCount = GetValuesPerBin (bodyProperty, bin);
	    if (countSoFar + currentCount > limit)
		break;
	    countSoFar += currentCount;
	    ++bin;
	}
	double ratio = (limit - countSoFar) / currentCount;
	double intervalSize = 
	    (GetMax (bodyProperty)  - GetMin (bodyProperty)) / 
	    histogramIntervals;
	double lowerValue = GetMin (bodyProperty) + bin * intervalSize;
	m_median[bodyProperty] = lowerValue + intervalSize * ratio;
    }
}
