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

bool StripIterator::HasNext () const
{
    return m_timeStep < m_bodyAlongTime.GetBodies ().size ();
}


StripIterator::StripPoint StripIterator::Next ()
{
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
	const boost::shared_ptr<Body>& body = 
	    m_bodyAlongTime.GetBody (m_timeStep);
	++m_timeStep;
	return StripPoint (body->GetCenter (), location, body);
    }
    else
    { // at the end of a middle wrap
	m_isNextBeginOfStrip = true;
	const OOBox& originalDomain = 
	    m_foamAlongTime.GetFoam (m_timeStep)->GetOriginalDomain ();
	const boost::shared_ptr<Body>& body = 
	    m_bodyAlongTime.GetBody (m_timeStep);
	return StripPoint (
	    originalDomain.TorusTranslate (
		body->GetCenter (),
		Vector3int16Zero - 
		m_bodyAlongTime.GetTranslation (m_currentWrap++)), END,
	    body);
    }
}

float StripIterator::GetColorByValue (
    CenterPathColor::Enum colorBy,
    const StripPoint& p, const StripPoint& prev)
{
    RuntimeAssert (p.m_location != BEGIN, "Invalid strip piece");
    G3D::Vector3 speedVector = p.m_point - prev.m_point;
    switch (colorBy)
    {
    case CenterPathColor::SPEED_ALONG_X:
	return speedVector.x;
    case CenterPathColor::SPEED_ALONG_Y:
	return speedVector.y;
    case CenterPathColor::SPEED_ALONG_Z:
	return speedVector.z;
    case CenterPathColor::SPEED_TOTAL:
	return speedVector.length ();
    case CenterPathColor::NONE:
	return 0;
    default:
	ThrowException ("Invalid speed: ", colorBy);
    }
    return 0;
}

float StripIterator::GetColorByValue (CenterPathColor::Enum colorBy,
				      const StripPoint& p)
{
    return p.m_body->GetRealAttribute (
	colorBy - CenterPathColor::PER_BODY_BEGIN);
}

bool StripIterator::ExistsColorByValue (CenterPathColor::Enum colorBy,
					const StripPoint& p)
{
    return p.m_body->ExistsAttribute (
	colorBy - CenterPathColor::PER_BODY_BEGIN);
}
