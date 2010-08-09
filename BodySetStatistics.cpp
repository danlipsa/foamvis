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
    m_min (BodyProperty::COUNT, numeric_limits<float> ().max ()),
    m_max (BodyProperty::COUNT, -numeric_limits<float> ().max ()),
    m_histogram (BodyProperty::COUNT)
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
	valuePerInterval (i, speedComponents[i]);
}

void BodySetStatistics::HistogramStep (const boost::shared_ptr<Body>& body)
{
    for (size_t i = BodyProperty::PER_BODY_BEGIN;
	 i < BodyProperty::PER_BODY_END; ++i)
    {
	size_t index = i - BodyProperty::PER_BODY_BEGIN;
	if (body->ExistsAttribute (index))
	    valuePerInterval (i, body->GetRealAttribute (index));
    }
}

void BodySetStatistics::HistogramStep (
    const FoamAlongTime& foamAlongTime, size_t bodyId, size_t timeStep)
{
    for (size_t i = BodyProperty::ENUM_BEGIN; i < BodyProperty::COUNT; ++i)
    {
	BodyProperty::Enum bodyProperty = BodyProperty::FromSizeT(i);
	if (foamAlongTime.ExistsBodyProperty (bodyProperty, bodyId, timeStep))
	    valuePerInterval (
		i, foamAlongTime.GetBodyProperty (
		    bodyProperty, bodyId, timeStep));
    }
}


void BodySetStatistics::valuePerInterval (size_t i, float value)
{
    float beginInterval = GetMin (i);
    float endInterval = GetMax (i);
    size_t bin;
    if (beginInterval == endInterval)
	bin = 0;
    else
    {
	float step = (endInterval - beginInterval) / HISTOGRAM_INTERVALS;
	bin = floor ((value - beginInterval) / step);
	if (bin == HISTOGRAM_INTERVALS)
	    bin = HISTOGRAM_INTERVALS - 1;
    }
    ++m_histogram[i][bin];
}

void BodySetStatistics::RangeStep (size_t bodyProperty, float newValue)
{
    MinStep (bodyProperty, newValue);
    MaxStep (bodyProperty, newValue);
}

QwtIntervalData BodySetStatistics::GetHistogram (size_t i) const
{
    QwtArray<QwtDoubleInterval> intervals (HISTOGRAM_INTERVALS);
    QwtArray<double> values (HISTOGRAM_INTERVALS);
    float beginInterval = GetMin (i);
    float endInterval = GetMax (i);
    float step = (endInterval - beginInterval) / HISTOGRAM_INTERVALS;
    float pos = beginInterval;
    for (size_t bin = 0; bin < HISTOGRAM_INTERVALS; ++bin)
    {
	intervals[bin] = QwtDoubleInterval (pos, pos + step);
	values[bin] = GetValuesPerBin (i, bin);
	pos += step;
    }
    return QwtIntervalData (intervals, values);
}
