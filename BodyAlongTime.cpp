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
#include "Simulation.h"
#include "Foam.h"
#include "Utils.h"


// Private Functions
// ======================================================================


// BodyAlongTime Methods
// ======================================================================

BodyAlongTime::BodyAlongTime (size_t timeSteps) :
    m_bodyAlongTime (timeSteps),
    /* set the times to invalid values */
    m_timeBegin (timeSteps),
    m_timeEnd (0)
{
}

void BodyAlongTime::SetBody (
    size_t timeStep, const boost::shared_ptr<Body>& body)
{
    m_bodyAlongTime[timeStep] = body;
    if (timeStep < m_timeBegin)
	m_timeBegin = timeStep;
    if (timeStep >= m_timeEnd)
	m_timeEnd = timeStep + 1;
}


void BodyAlongTime::CalculateBodyWraps (const Simulation& simulation)
{
    if (simulation.IsTorus ())
    {
	for (size_t time = m_timeBegin; time < (m_timeEnd - 1); time++)
	{
	    const OOBox& originalDomain = 
		simulation.GetFoam (time+1).GetOriginalDomain ();
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

string BodyAlongTime::ToString () const
{
    ostringstream ostr;
    ostr << "BodyAlongTime " << GetId () << ": " << endl;
    
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

void BodyAlongTime::AssertDeadBubblesStayDead () const
{
    for (size_t i = m_timeBegin; i < m_timeEnd; ++i)
    {
	if (m_bodyAlongTime[i] == 0)
	{
	    size_t j = i + 1;
	    while (j < m_timeEnd && m_bodyAlongTime[j] == 0)
		++j;
	    RuntimeAssert (false, 
			   "Body with id (0 based) ", 
			   m_bodyAlongTime[m_timeBegin]->GetId (),
			   " is null at time step ", i, 
			   " and then non-null at timestep ", j);
   	}
    }
}

size_t BodyAlongTime::GetId () const
{
    return m_bodyAlongTime[m_timeBegin]->GetId ();
}




// BodiesAlongTime Methods
// ======================================================================
BodiesAlongTime::BodiesAlongTime ()
{
}

BodyAlongTime& BodiesAlongTime::getBodyAlongTime (size_t id) const
{
    BodyMap::const_iterator it = m_bodyMap.find (id);
    RuntimeAssert (it != m_bodyMap.end (),
		   "Body not found: ", id);
    return *(it->second);
}

void BodiesAlongTime::AllocateBodyAlongTime (size_t bodyId, size_t timeSteps)
{
    boost::shared_ptr<BodyAlongTime> oneBodyPtr (new BodyAlongTime (timeSteps));
    pair<BodyMap::iterator, bool> result = 
	m_bodyMap.insert (BodyMap::value_type (bodyId, oneBodyPtr));
}

void BodiesAlongTime::CacheBody (
    boost::shared_ptr<Body> body, size_t timeStep, size_t timeSteps)
{
    size_t id = body->GetId ();
    // Bubbles might be created at later times steps.
    if (m_bodyMap.find (id) == m_bodyMap.end ())
	AllocateBodyAlongTime (id, timeSteps);
    m_bodyMap[id]->SetBody(timeStep, body);
}

string BodiesAlongTime::ToString () const
{
    ostringstream ostr;
    for (BodiesAlongTime::BodyMap::const_iterator it = m_bodyMap.begin ();
	 it != m_bodyMap.end(); ++it)
	ostr << *(it->second) << endl;
    return ostr.str ();
}

void BodiesAlongTime::AssertDeadBubblesStayDead () const
{
    for (BodiesAlongTime::BodyMap::const_iterator it = m_bodyMap.begin ();
	 it != m_bodyMap.end(); ++it)
	it->second->AssertDeadBubblesStayDead ();
}
