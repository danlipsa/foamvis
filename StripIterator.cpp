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
    if (m_currentWrap == m_bodyAlongTime.GetWraps ().size () ||
	m_timeStep < m_bodyAlongTime.GetWraps ()[m_currentWrap] + 1)
    {
	bool wasNewStrip = m_newStrip;
	m_newStrip = false;
	return StripPoint (
	    m_bodyAlongTime.GetBody (m_timeStep++)->GetCenter (), wasNewStrip);
    }
    else
    {
	const OOBox& originalDomain = 
	    m_foamAlongTime.GetFoam (m_timeStep)->GetOriginalDomain ();
	m_newStrip = true;
	return StripPoint (
	    originalDomain.TorusTranslate (
		m_bodyAlongTime.GetBody (m_timeStep)->GetCenter (),
		Vector3int16Zero - 
		m_bodyAlongTime.GetTranslation (m_currentWrap++)), false);
    }
}
