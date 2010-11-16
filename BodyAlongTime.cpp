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


// BodyAlongTime Methods
// ======================================================================

BodyAlongTime::BodyAlongTime (size_t timeSteps) :
    BodySetStatistics (),
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
    const StripIterator::Point& beforeBegin,
    const StripIterator::Point& begin,
    const StripIterator::Point& end,
    const StripIterator::Point& afterEnd)
{
    static_cast<void>(beforeBegin);
    static_cast<void>(afterEnd);
    G3D::Vector3 speed = end.m_point - begin.m_point;
    // Warning: should have the same ordering as BodyProperty::Enum
    boost::array<double, 4> speedComponents = 
	{{speed.x, speed.y, speed.z, speed.length ()}};
    for (size_t i = BodyProperty::VELOCITY_BEGIN;
	 i < BodyProperty::VELOCITY_END; ++i)
	RangeStep (i, speedComponents[i]);
}

void BodyAlongTime::rangeStep (const boost::shared_ptr<Body>& body)
{
    for (size_t i = BodyProperty::PER_BODY_BEGIN;
	 i < BodyProperty::PER_BODY_END; ++i)
	rangeStep (body, BodyProperty::FromSizeT (i));
}


void BodyAlongTime::rangeStep (const boost::shared_ptr<Body>& body,
			       BodyProperty::Enum property)
{
    size_t index = property - BodyProperty::PER_BODY_BEGIN;
    if (body->ExistsAttribute (index))
	RangeStep (property, body->GetRealAttribute (index));
}

void BodyAlongTime::CalculateRange (const FoamAlongTime& foamAlongTime)
{
    // per segment values (speeds)
    StripIterator it = GetStripIterator (foamAlongTime);
    it.ForEachSegment (
	boost::bind (&BodyAlongTime::speedRangeStep, this, _1, _2, _3, _4));
    // per time step values
    for_each (m_bodyAlongTime.begin (), m_bodyAlongTime.end (),
	      boost::bind (&BodyAlongTime::rangeStep, this, _1));
    NormalizeEmptyRange ();
}

void BodyAlongTime::CalculateRange (BodyProperty::Enum bodyProperty)
{
    // per time step values
    for_each (m_bodyAlongTime.begin (), m_bodyAlongTime.end (),
	      boost::bind (
		  &BodyAlongTime::rangeStep, this, _1, bodyProperty));
    NormalizeEmptyRange ();
}



void BodyAlongTime::CalculateHistogram (
    const FoamAlongTime& foamAlongTime, BodySetStatistics* destination)
{
    // per segment values (speeds)
    StripIterator it = GetStripIterator (foamAlongTime);
    it.ForEachSegment (
	boost::bind (&BodyAlongTime::SpeedHistogramStep, 
		     destination, _1, _2, _3, _4));
    // per time step values
    for_each (GetBodies ().begin (), GetBodies ().end (),
	      boost::bind (&BodyAlongTime::HistogramStep, 
			   destination, _1));
}

size_t BodyAlongTime::GetId () const
{
    return m_bodyAlongTime[0]->GetId ();
}




// BodiesAlongTime Methods
// ======================================================================
BodiesAlongTime::BodiesAlongTime () :
    BodySetStatistics ()
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
    RuntimeAssert (m_bodyMap[id] != 0, 
		   "Body with id: ", id, 
		   " (0 based) not defined in the first DMP file");
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

void BodiesAlongTime::CalculateOverallRange (const FoamAlongTime& foamAlongTime)
{
    BOOST_FOREACH (BodyMap::value_type p, GetBodyMap ())
    {
	BodyAlongTime& bat = *p.second;
	bat.CalculateRange (foamAlongTime);
	for (size_t bodyProperty = 0; bodyProperty < Size (); ++bodyProperty)
	{
	    MinStep (bodyProperty, bat.GetMin (bodyProperty));
	    MaxStep (bodyProperty, bat.GetMax (bodyProperty));
	}
    }
    NormalizeEmptyRange ();
}

void BodiesAlongTime::CalculateOverallHistogram (
    const FoamAlongTime& foamAlongTime)
{
    BOOST_FOREACH (BodyMap::value_type p, GetBodyMap ())
	p.second->CalculateHistogram (foamAlongTime, this);
}
