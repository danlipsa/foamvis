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


// Private Classes
// ======================================================================
struct squareDeviation
{
public:
    squareDeviation (float mean) : m_mean (mean) {}
    float operator() (float acc, float x)
    {
	float deviation = x - m_mean;
	return acc + deviation * deviation;
    }
private:
    float m_mean;
};


// Private Functions
// ======================================================================
bool isNull (const boost::shared_ptr<Body>  body)
{
    return body == 0;
}


// BodiesAlongTime Methods
// ======================================================================
BodiesAlongTime::OneBody& BodiesAlongTime::GetOneBody (size_t id)
{
    BodyMap::iterator it = m_bodyMap.find (id);
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
    using G3D::Vector3;
    vector<float> centerMovement (m_bodyAlongTime.size () - 1);
    for (size_t time = 0; time < m_bodyAlongTime.size () - 1; time++)
    {
	const Vector3& begin = m_bodyAlongTime[time]->GetCenter ();
	const Vector3& end = m_bodyAlongTime[time + 1]->GetCenter ();
	centerMovement[time] = (end - begin).length ();
    }

    for (size_t time = 0; time < centerMovement.size (); time++)
    {
	const OOBox& originalDomain = 
	    foamAlongTime.GetFoam (time)->GetOriginalDomain ();
	float minDomainSide = min (originalDomain.GetX ().length (),
				   min (originalDomain.GetY ().length(),
					originalDomain.GetZ ().length ()));
	if (centerMovement[time] > minDomainSide / 2)
	    m_wraps.push_back (time);
    }
    m_wraps.push_back (centerMovement.size ());
}

void BodyAlongTime::Resize ()
{
    Bodies::iterator it = find_if (
	m_bodyAlongTime.begin (), m_bodyAlongTime.end (), isNull);
    size_t size = it - m_bodyAlongTime.begin ();
    m_bodyAlongTime.resize (size);
}

// Static and Friends BodyAlongTime Methods
// ======================================================================
ostream& operator<< (
    ostream& ostr, const BodyAlongTime& bat)
{
    const BodyAlongTime::Bodies& bodies = bat.GetBodies ();
    ostr << "BodyAlongTime " << bodies[0]->GetId () 
	 << ": " << endl;
    
    ostr << "Wraps: ";
    ostream_iterator<size_t> os (ostr, " ");
    copy (bat.m_wraps.begin (), bat.m_wraps.end (), os);
    ostr << endl;
    return ostr;
}
