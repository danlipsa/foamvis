/**
 * @file FoamAlongTime.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Foam objects
 */

#include "Body.h"
#include "Debug.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "Utils.h"

// Private Functions
// ======================================================================
void calculateBodyWraps (BodiesAlongTime::BodyMap::value_type& v,
    const FoamAlongTime& foamAlongTime)
{
    v.second->CalculateBodyWraps (foamAlongTime);
}


// Members
// ======================================================================
void FoamAlongTime::Calculate (
    Aggregate aggregate, FoamLessThanAlong::Corner corner, G3D::Vector3& v)
{
    using G3D::Vector3;
    Foams::iterator it;
    it = aggregate (m_foams.begin (), m_foams.end (), 
		    FoamLessThanAlong(Vector3::X_AXIS, corner));
    v.x = ((*it).get()->*corner) ().x;

    it = aggregate (m_foams.begin (), m_foams.end (), 
	    FoamLessThanAlong(Vector3::Y_AXIS, corner));    
    v.y = ((*it).get()->*corner) ().y;

    it = aggregate (m_foams.begin (), m_foams.end (), 
	    FoamLessThanAlong(Vector3::Z_AXIS, corner));
    v.z = ((*it).get()->*corner) ().z;
}

void FoamAlongTime::CalculateAABox ()
{
    using G3D::Vector3;
    Vector3 low, high;
    Calculate (min_element, &Foam::GetAABoxLow, low);
    Calculate (max_element, &Foam::GetAABoxHigh, high);
    m_AABox.set (low, high);
}

void FoamAlongTime::CalculateBodyWraps ()
{
    if (m_foams.size () > 1)
    {
	BodiesAlongTime::BodyMap bodyMap = GetBodiesAlongTime ().GetBodyMap ();
	for_each (bodyMap.begin (), bodyMap.end (),
		  boost::bind(calculateBodyWraps, _1, *this));
    }
}

void FoamAlongTime::PostProcess ()
{
    CalculateAABox ();
    CacheBodiesAlongTime ();
    CalculateBodyWraps ();
    GetBodiesAlongTime ().CalculateValueRange (*this);
    GetBodiesAlongTime ().CalculateHistogram (*this);
}

void FoamAlongTime::CacheBodiesAlongTime ()
{
    Foam::Bodies& bodies = m_foams[0]->GetBodies ();
    for_each (bodies.begin (), bodies.end (), 
	      boost::bind (&BodiesAlongTime::Allocate,
			   &m_bodiesAlongTime, _1, m_foams.size ()));
    for (size_t timeStep = 0; timeStep < m_foams.size (); timeStep++)
    {
	Foam::Bodies& bodies = m_foams[timeStep]->GetBodies ();
	BOOST_FOREACH (boost::shared_ptr<Body>  body, bodies)
	    m_bodiesAlongTime.Cache (body, timeStep);
    }
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&BodiesAlongTime::Resize, &m_bodiesAlongTime, _1));
}

float FoamAlongTime::GetBodyProperty (
    BodyProperty::Enum property,
    size_t bodyId, size_t timeStep) const
{
    const BodyAlongTime& bat = GetBodiesAlongTime ().GetBodyAlongTime (bodyId);
    if (property >= BodyProperty::VELOCITY_BEGIN &&
	property < BodyProperty::VELOCITY_END)
    {
	if (timeStep == m_foams.size () - 1)
	    --timeStep;
	StripIterator it (bat, *this, timeStep);
	RuntimeAssert (it.HasNext (),
		       "Cannot find velocity. First point not available");
	StripIterator::Point p = it.Next ();
	RuntimeAssert (it.HasNext (), 
		       "Cannot find velocity. Second point not available");
	StripIterator::Point next = it.Next ();
	return StripIterator::GetPropertyValue (property, next, p);
    }
    else
	return bat.GetBody (timeStep)->GetPropertyValue (property);
}


bool FoamAlongTime::ExistsBodyProperty (
    BodyProperty::Enum property,
    size_t bodyId, size_t timeStep) const
{
    if (property >= BodyProperty::VELOCITY_BEGIN &&
	property < BodyProperty::VELOCITY_END)
    {
	if (GetTimeSteps () < 2)
	    return false;
	else
	    return true;
    }
    const BodyAlongTime& bat = GetBodiesAlongTime ().GetBodyAlongTime (bodyId);
    return bat.GetBody (timeStep)->ExistsPropertyValue (property);
}


// Static and Friend Members
//======================================================================

ostream& operator<< (ostream& ostr, const FoamAlongTime& dataAlongTime)
{
    ostr << "FoamAlongTime: " << endl;
    ostr << dataAlongTime.m_AABox << endl;
    ostream_iterator< boost::shared_ptr<const Foam> > output (ostr, "\n");
    copy (dataAlongTime.m_foams.begin (), dataAlongTime.m_foams.end (), output);
    ostr << dataAlongTime.m_bodiesAlongTime;
    return ostr;
}

