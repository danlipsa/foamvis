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


const size_t HISTOGRAM_INTERVALS = 256;

BodySetStatistics::BodySetStatistics () :
    m_min (BodyProperty::COUNT, numeric_limits<float> ().max ()),
    m_max (BodyProperty::COUNT, -numeric_limits<float> ().max ()),
    m_histogram (BodyProperty::COUNT),
    m_maxCountPerBin (BodyProperty::COUNT)
{
    BOOST_FOREACH (vector<size_t>& v, m_histogram)
	v.resize (HISTOGRAM_INTERVALS, 0);
}


void BodySetStatistics::SpeedHistogramStep (
    const StripIterator::Point& p,
    const StripIterator::Point& prev)
{
    G3D::Vector3 speed = p.m_point - prev.m_point;
    boost::array<float, 4> speedComponents = 
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
    for (size_t i = BodyProperty::ENUM_BEGIN; i < BodyProperty::COUNT; ++i)
    {
	BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT(i);
	if (foamAlongTime.ExistsBodyProperty (bodyProperty, bodyId, timeStep))
	    valuePerInterval (
		i, foamAlongTime.GetBodyProperty (
		    bodyProperty, bodyId, timeStep),
		rangeStatistics.GetMin (i), rangeStatistics.GetMax (i));
    }
}


void BodySetStatistics::valuePerInterval (size_t i, float value, 
					  float beginInterval, float endInterval)
{
    size_t bin = GetBin (
	value, HistogramIntervals (), beginInterval, endInterval);
    ++m_histogram[i][bin];
}

size_t BodySetStatistics::GetBin (
    float value, size_t binCount, float beginInterval, float endInterval)
{
    if (beginInterval == endInterval || value < beginInterval)
	return 0;
    else if (value >= endInterval)
	return binCount - 1;
    else
    {
	float step = (endInterval - beginInterval) / binCount;
	return floor ((value - beginInterval) / step);
    }
}


void BodySetStatistics::RangeStep (size_t bodyProperty, float newValue)
{
    MinStep (bodyProperty, newValue);
    MaxStep (bodyProperty, newValue);
}

QwtIntervalData BodySetStatistics::GetHistogram (
    size_t i, const BodySetStatistics* rangeStatistics) const
{
    if (rangeStatistics == 0)
	rangeStatistics = this;
    size_t histogramIntervals = HistogramIntervals ();
    QwtArray<QwtDoubleInterval> intervals (histogramIntervals);
    QwtArray<double> values (histogramIntervals);
    float beginInterval = rangeStatistics->GetMin (i);
    float endInterval = rangeStatistics->GetMax (i);
    float step = (endInterval - beginInterval) / histogramIntervals;
    float pos = beginInterval;
    for (size_t bin = 0; bin < histogramIntervals; ++bin)
    {
	intervals[bin] = QwtDoubleInterval (pos, pos + step);
	values[bin] = GetValuesPerBin (i, bin);
	pos += step;
    }
    return QwtIntervalData (intervals, values);
}

void BodySetStatistics::CalculateMaxCountPerBin ()
{
    for (size_t bodyProperty = BodyProperty::ENUM_BEGIN;
	 bodyProperty < BodyProperty::COUNT; ++bodyProperty)
    {
	size_t maxCount = 0;
	size_t histogramIntervals = HistogramIntervals ();
	for (size_t bin = 0; bin < histogramIntervals; ++bin)
	    maxCount = max (maxCount, GetValuesPerBin (bodyProperty, bin));
	m_maxCountPerBin[bodyProperty] = maxCount;
    }
}
