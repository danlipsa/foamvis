/**
 * @file   BodyAlongTime.cpp
 * @author Dan R. Lipsa
 * @date 15 May 2010
 *
 * Implementation of the bodies along time class
 */

#include "Body.h"
#include "BodyAlongTime.h"
#include "FoamAlongTime.h"
#include "Foam.h"
#include "Debug.h"
#include "DebugStream.h"


// Private Functions
// ======================================================================
bool isNull (const boost::shared_ptr<Body>  body)
{
    return body == 0;
}


// BodiesAlongTime Methods
// ======================================================================
BodiesAlongTime::OneBody& BodiesAlongTime::getOneBody (size_t id) const
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
    OneBodyPtr oneBodyPtr (new OneBody (timeSteps));
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
    GetOneBody (body->GetId ()).Resize ();
}

void BodiesAlongTime::resize (size_t id, size_t timeSteps)
{
    m_bodyMap[id]->Resize (timeSteps);
}

// Static and Friends BodiesAlongTime Methods
// ======================================================================
ostream& operator<< (
    ostream& ostr, const BodiesAlongTime& bat)
{
    const BodiesAlongTime::BodyMap& bm = bat.m_bodyMap;
    for (BodiesAlongTime::BodyMap::const_iterator it = bm.begin ();
	 it != bm.end(); ++it)
	ostr << *(it->second) << endl;
    return ostr;
}


// BodyAlongTime Methods
// ======================================================================
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
    m_wraps.push_back (m_bodyAlongTime.size () - 1);
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

