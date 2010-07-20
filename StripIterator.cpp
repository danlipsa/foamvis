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
	return StripPoint (
	    m_bodyAlongTime.GetBody (m_timeStep++)->GetCenter (), location);
    }
    else
    { // at the end of a middle wrap
	m_isNextBeginOfStrip = true;
	const OOBox& originalDomain = 
	    m_foamAlongTime.GetFoam (m_timeStep)->GetOriginalDomain ();
	return StripPoint (
	    originalDomain.TorusTranslate (
		m_bodyAlongTime.GetBody (m_timeStep)->GetCenter (),
		Vector3int16Zero - 
		m_bodyAlongTime.GetTranslation (m_currentWrap++)), END);
    }
}

float StripIterator::GetColorByValue (
    const StripPoint& p, const StripPoint& prev) const
{
    RuntimeAssert (p.m_location != BEGIN, "Invalid strip piece");
    G3D::Vector3 speedVector = p.m_point - prev.m_point;
    switch (m_colorBy)
    {
    case CenterPathColor::SPEED_ALONG_X:
	return speedVector.x;
    case CenterPathColor::SPEED_ALONG_Y:
	return speedVector.y;
    case CenterPathColor::SPEED_ALONG_Z:
	return speedVector.z;
    case CenterPathColor::SPEED_TOTAL:
	return speedVector.length ();
    default:
	return 0;
    }
}

