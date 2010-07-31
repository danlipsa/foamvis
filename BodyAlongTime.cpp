/**
 * @file   BodyAlongTime.cpp
 * @author Dan R. Lipsa
 * @date 15 May 2010
 *
 * Implementation of the bodies along time class
 */

#include "Body.h"
#include "BodyAlongTime.h"
#include "Debug.h"
#include "DebugStream.h"
#include "FoamAlongTime.h"
#include "Foam.h"
#include "Utils.h"


// Private Functions
// ======================================================================
bool isNull (const boost::shared_ptr<Body>& body)
{
    return body == 0;
}


// BodyAlongTimeStatistics Methods
// ======================================================================
BodyAlongTimeStatistics::BodyAlongTimeStatistics () :
    m_min (CenterPathColor::COUNT, numeric_limits<float> ().max ()),
    m_max (CenterPathColor::COUNT, numeric_limits<float> ().min ()),
    m_valuesPerInterval (CenterPathColor::COUNT)
{
    BOOST_FOREACH (vector<size_t>& v, m_valuesPerInterval)
	v.resize (HISTOGRAM_INTERVALS, 0);
}


// BodyAlongTime Methods
// ======================================================================

BodyAlongTime::BodyAlongTime (size_t timeSteps) :
    BodyAlongTimeStatistics (),
    m_bodyAlongTime (timeSteps)
{
}

void BodyAlongTime::CalculateBodyWraps (const FoamAlongTime& foamAlongTime)
{
    if (foamAlongTime.GetFoam (0)->IsTorus ())
    {
	for (size_t time = 0; time < (m_bodyAlongTime.size () - 1); time++)
	{
	    const OOBox& originalDomain = 
		foamAlongTime.GetFoam (time+1)->GetOriginalDomain ();
	    G3D::Vector3int16 translation;
	    const G3D::Vector3& begin = m_bodyAlongTime[time]->GetCenter ();
	    const G3D::Vector3& end = m_bodyAlongTime[time + 1]->GetCenter ();
	    if (originalDomain.IsWrap (begin, end, &translation))
	    {
		m_wraps.push_back (time);
		m_translations.push_back (translation);
	    }
	}
    }
}

void BodyAlongTime::Resize ()
{
    Bodies::iterator it = find_if (
	m_bodyAlongTime.begin (), m_bodyAlongTime.end (), isNull);
    size_t size = it - m_bodyAlongTime.begin ();
    m_bodyAlongTime.resize (size);
}

string BodyAlongTime::ToString () const
{
    ostringstream ostr;
    const BodyAlongTime::Bodies& bodies = GetBodies ();
    ostr << "BodyAlongTime " << bodies[0]->GetId () << ": " << endl;
    
    ostr << "Wraps: ";
    ostream_iterator<size_t> os (ostr, " ");
    copy (m_wraps.begin (), m_wraps.end (), os);
    ostr << endl;

    ostr << "Translations: ";
    ostream_iterator<G3D::Vector3int16> ov (ostr, " ");
    copy (m_translations.begin (), m_translations.end (), ov);
    ostr << endl;
    return ostr.str ();
}

void BodyAlongTime::speedRangeStep (
    const StripIterator::StripPoint& p,
    const StripIterator::StripPoint& prev)
{
    G3D::Vector3 speed = p.m_point - prev.m_point;
    boost::array<float, 4> speedComponents = 
	{{speed.x, speed.y, speed.z, speed.length ()}};
    for (size_t i = 0; i < m_min.size (); ++i)
    {
	m_min[i] = min (m_min[i], speedComponents[i]);
	m_max[i] = max (m_max[i], speedComponents[i]);
    }
}

void BodyAlongTime::rangeStep (
    const boost::shared_ptr<Body>& body)
{
    m_min[CenterPathColor::PRESSURE] = 
	min (m_min[CenterPathColor::PRESSURE], body->GetPressure ());
    m_max[CenterPathColor::PRESSURE] = 
	max (m_max[CenterPathColor::PRESSURE], body->GetPressure ());
}


void BodyAlongTime::speedValuesPerIntervalStep (
    const StripIterator::StripPoint& p,
    const StripIterator::StripPoint& prev)
{
    G3D::Vector3 speed = p.m_point - prev.m_point;
    boost::array<float, 4> speedComponents = 
	{{speed.x, speed.y, speed.z, speed.length ()}};
    for (size_t i = 0; i < speedComponents.size (); ++i)
	valuePerInterval (i, speedComponents[i]);
}

void BodyAlongTime::valuesPerIntervalStep (const boost::shared_ptr<Body>& body)
{
    valuePerInterval (CenterPathColor::PRESSURE, body->GetPressure ());
}


void BodyAlongTime::valuePerInterval (size_t i, float value)
{
    float beginInterval = GetMin (i);
    float endInterval = GetMax (i);
    float step = (endInterval - beginInterval) / HISTOGRAM_INTERVALS;
    size_t bin = floor ((value - beginInterval) / step);
    if (bin == HISTOGRAM_INTERVALS)
	bin = HISTOGRAM_INTERVALS - 1;
    ++m_valuesPerInterval[i][bin];
}


void BodyAlongTime::CalculateValueRange (const FoamAlongTime& foamAlongTime)
{
    // per segment values (speeds)
    StripIterator it = GetStripIterator (foamAlongTime);
    it.ForEachSegment (
	boost::bind (&BodyAlongTime::speedRangeStep, this, _1, _2));
    // per time step values
    for_each (m_bodyAlongTime.begin (), m_bodyAlongTime.end (),
	      boost::bind (&BodyAlongTime::rangeStep, this, _1));
}

void BodyAlongTime::CalculateValuesPerInterval (
    const FoamAlongTime& foamAlongTime)
{
    // per segment values (speeds)
    StripIterator it = GetStripIterator (foamAlongTime);
    it.ForEachSegment (
	boost::bind (&BodyAlongTime::speedValuesPerIntervalStep, this, _1, _2));
    // per time step values
    for_each (m_bodyAlongTime.begin (), m_bodyAlongTime.end (),
	      boost::bind (&BodyAlongTime::valuesPerIntervalStep, this, _1));
}

size_t BodyAlongTime::GetId () const
{
    return m_bodyAlongTime[0]->GetId ();
}



// BodiesAlongTime Methods
// ======================================================================
BodiesAlongTime::BodiesAlongTime () :
    BodyAlongTimeStatistics ()
{
}

BodyAlongTime& BodiesAlongTime::getBodyAlongTime (size_t id) const
{
    BodyMap::const_iterator it = m_bodyMap.find (id);
    RuntimeAssert (it != m_bodyMap.end (),
		   "Body not found: ", id);
    return *(it->second);
}

void BodiesAlongTime::Allocate (
    const boost::shared_ptr<Body>  body, size_t timeSteps)
{
    size_t id = body->GetId ();
    boost::shared_ptr<BodyAlongTime> oneBodyPtr (new BodyAlongTime (timeSteps));
    m_bodyMap.insert (
	BodyMap::value_type (id, oneBodyPtr));
}

void BodiesAlongTime::Cache (boost::shared_ptr<Body>  body, size_t timeStep)
{
    size_t id = body->GetId ();
    m_bodyMap[id]->GetBody(timeStep) = body;
}

void BodiesAlongTime::Resize (const boost::shared_ptr<Body>  body)
{
    GetBodyAlongTime (body->GetId ()).Resize ();
}

void BodiesAlongTime::resize (size_t id, size_t timeSteps)
{
    m_bodyMap[id]->Resize (timeSteps);
}

string BodiesAlongTime::ToString () const
{
    ostringstream ostr;
    for (BodiesAlongTime::BodyMap::const_iterator it = m_bodyMap.begin ();
	 it != m_bodyMap.end(); ++it)
	ostr << *(it->second) << endl;
    return ostr.str ();
}

void BodiesAlongTime::CalculateValueRange (const FoamAlongTime& foamAlongTime)
{
    BOOST_FOREACH (BodyMap::value_type p, GetBodyMap ())
    {
	BodyAlongTime& bat = *p.second;
	bat.CalculateValueRange (foamAlongTime);
	for (size_t i = 0; i < m_min.size (); i++)
	{
	    m_min[i] = min (m_min[i], bat.GetMin (i));
	    m_max[i] = max (m_max[i], bat.GetMax (i));
	}
    }
}

void BodiesAlongTime::CalculateValuesPerInterval (
    const FoamAlongTime& foamAlongTime)
{
    BOOST_FOREACH (BodyMap::value_type p, GetBodyMap ())
    {
	BodyAlongTime& bat = *p.second;
	bat.CalculateValuesPerInterval (foamAlongTime);
	for (size_t i = 0; i < m_min.size (); i++)
	{
	    for (size_t bin = 0; bin < HISTOGRAM_INTERVALS; ++bin)
	    {
		size_t valuesPerBin = bat.GetValuesPerInterval(i, bin);
		m_valuesPerInterval[i][bin] += valuesPerBin;
	    }
	}
    }
}

QwtIntervalData BodiesAlongTime::GetValuesPerInterval (
    size_t speedComponent) const
{
    QwtArray<QwtDoubleInterval> intervals (HISTOGRAM_INTERVALS);
    QwtArray<double> values (HISTOGRAM_INTERVALS);
    float beginInterval = GetMin (speedComponent);
    float endInterval = GetMax (speedComponent);
    float step = (endInterval - beginInterval) / HISTOGRAM_INTERVALS;
    float pos = beginInterval;
    for (size_t bin = 0; bin < HISTOGRAM_INTERVALS; ++bin)
    {
	intervals[bin] = QwtDoubleInterval (pos, pos + step);
	values[bin] = 
	    BodyAlongTimeStatistics::GetValuesPerInterval (
		speedComponent, bin);
	pos += step;
    }
    return QwtIntervalData (intervals, values);
}
