/**
 * @file   StripIterator.cpp
 * @author Dan R. Lipsa
 * @date 16 July 2010
 *
 * Implementation of the StripIterator class
 */

#include "Body.h"
#include "BodyAlongTime.h"
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
	float colorByValue;
	Location location;
	if (// at the end of last wrap
	    m_timeStep == m_bodyAlongTime.GetBodies ().size () - 1)
	{
	    colorByValue = 0;
	    location = END;
	    m_isNextBeginOfStrip = false;
	}
	else
	{   // not at the end of a middle wrap
	    colorByValue = getColorByValue ();
	    location = m_isNextBeginOfStrip ? BEGIN : MIDDLE;
	    m_isNextBeginOfStrip = false;
	}
	return StripPoint (
	    m_bodyAlongTime.GetBody (m_timeStep++)->GetCenter (), location,
	    colorByValue);
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
		m_bodyAlongTime.GetTranslation (m_currentWrap++)), END, 0);
    }
}

float StripIterator::getColorByValue () const
{
    switch (m_colorBy)
    {
    case CenterPathColorBy::SPEED_ALONG_X:
	return getSpeedVector ().x;
    case CenterPathColorBy::SPEED_ALONG_Y:
	return getSpeedVector ().y;
    case CenterPathColorBy::SPEED_ALONG_Z:
	return getSpeedVector ().z;
    case CenterPathColorBy::SPEED_TOTAL:
	return getSpeedVector ().length ();
    default:
	return 0;
    }
}

G3D::Vector3 StripIterator::getSpeedVector () const
{
    return m_bodyAlongTime.GetBody (m_timeStep + 1)->GetCenter () - 
	m_bodyAlongTime.GetBody (m_timeStep)->GetCenter ();
}
