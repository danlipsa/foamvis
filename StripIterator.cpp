/**
 * @file   StripIterator.cpp
 * @author Dan R. Lipsa
 * @date 16 July 2010
 *
 * Implementation of the StripIterator class
 */

#include "Body.h"
#include "BodyAlongTime.h"
#include "Debug.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "OOBox.h"
#include "StripIterator.h"
#include "Utils.h"

StripIterator::StripIterator (
    const BodyAlongTime& bodyAlongTime,
    const FoamAlongTime& foamAlongTime, size_t timeStep) : 

    m_timeStep (timeStep),
    m_bodyAlongTime (bodyAlongTime),
    m_foamAlongTime (foamAlongTime)
{
    m_currentWrap = 0;
    size_t wrapSize = m_bodyAlongTime.GetWrapSize ();
    if (wrapSize == 0)
	m_isNextBeginOfStrip = (m_timeStep == 0);
    else
    {
	while (m_timeStep > m_bodyAlongTime.GetWrap (m_currentWrap) &&
	       m_currentWrap < wrapSize)
	    ++m_currentWrap;
	if (m_timeStep == 0 ||
	    m_timeStep == m_bodyAlongTime.GetWrap (m_currentWrap - 1) + 1)
	    m_isNextBeginOfStrip = true;
	else
	    m_isNextBeginOfStrip = false;
    }
}



bool StripIterator::HasNext () const
{
    return m_timeStep < m_bodyAlongTime.GetBodies ().size ();
}


StripIterator::Point StripIterator::Next ()
{
    Point point;
    boost::shared_ptr<Body> body;
    if (// last wrap
	m_currentWrap == m_bodyAlongTime.GetWrapSize () ||
	// not at the end of a middle wrap
	m_timeStep < m_bodyAlongTime.GetWrap (m_currentWrap) + 1)
    {
	Location location;
	if (// at the end of last wrap
	    m_timeStep == m_bodyAlongTime.GetBodies ().size () - 1)
	{
	    location = END;
	    m_isNextBeginOfStrip = false;
	}
	else
	{   // not at the end of a middle wrap or last wrap
	    location = m_isNextBeginOfStrip ? BEGIN : MIDDLE;
	    m_isNextBeginOfStrip = false;
	}
	body = m_bodyAlongTime.GetBody (m_timeStep);
	point = Point (body->GetCenter (), location, m_timeStep++, body);
    }
    else
    { // at the end of a middle wrap
	m_isNextBeginOfStrip = true;
	const OOBox& originalDomain = 
	    m_foamAlongTime.GetFoam (m_timeStep)->GetOriginalDomain ();
	body = m_bodyAlongTime.GetBody (m_timeStep);
	point = Point (
	    originalDomain.TorusTranslate (
		body->GetCenter (),
		Vector3int16Zero - m_bodyAlongTime.GetTranslation (
		    m_currentWrap++)), END, m_timeStep, body);
    }
    return point;
}

double StripIterator::GetVelocityValue (
    BodyProperty::Enum property,
    const Point& p, const Point& prev)
{
    RuntimeAssert (p.m_location != BEGIN, "Invalid strip piece");
    G3D::Vector3 speedVector = p.m_point - prev.m_point;
    switch (property)
    {
    case BodyProperty::VELOCITY_ALONG_X:
	return speedVector.x;
    case BodyProperty::VELOCITY_ALONG_Y:
	return speedVector.y;
    case BodyProperty::VELOCITY_ALONG_Z:
	return speedVector.z;
    case BodyProperty::VELOCITY_MAGNITUDE:
	return speedVector.length ();
    case BodyProperty::NONE:
	return 0;
    default:
	ThrowException ("Invalid speed: ", property);
    }
    return 0;
}

double StripIterator::GetPropertyValue (BodyProperty::Enum property,
				       const Point& p)
{
    return p.m_body->GetPropertyValue (property);
}

bool StripIterator::ExistsPropertyValue (BodyProperty::Enum property,
					const Point& p)
{
    return p.m_body->ExistsPropertyValue (property);
}
