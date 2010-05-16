/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 15 May 2010
 *
 * Implementation of the bodies along time class
 */

#include "Body.h"
#include "BodiesAlongTime.h"
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
bool isNull (const Body* body)
{
    return body == 0;
}


// BodiesAlongTime Methods
// ======================================================================
BodiesAlongTime::OneBody& BodiesAlongTime::GetOneBody (size_t originalIndex)
{
    BodyMap::iterator it = m_bodiesAlongTime.find (originalIndex);
    RuntimeAssert (it != m_bodiesAlongTime.end (),
		   "Body not found: ", originalIndex);
    return *(it->second);
}

void BodiesAlongTime::Allocate (const Body* body, size_t timeSteps)
{
    size_t originalIndex = body->GetOriginalIndex ();
    OneBodyPtr oneBodyPtr (new OneBody (timeSteps));
    m_bodiesAlongTime.insert (
	BodyMap::value_type (originalIndex, oneBodyPtr));
}

void BodiesAlongTime::Cache (Body* body, size_t timeStep)
{
    size_t originalIndex = body->GetOriginalIndex ();
    m_bodiesAlongTime[originalIndex]->GetBody(timeStep) = body;
}

void BodiesAlongTime::Resize (const Body* body)
{
    GetOneBody (body->GetOriginalIndex ()).Resize ();
}

void BodiesAlongTime::resize (size_t originalIndex, size_t timeSteps)
{
    m_bodiesAlongTime[originalIndex]->Resize (timeSteps);
}

// Static and Friends BodiesAlongTime Methods
// ======================================================================
ostream& operator<< (
    ostream& ostr, const BodiesAlongTime& bat)
{
    const BodiesAlongTime::BodyMap& bm = bat.m_bodiesAlongTime;
    for (BodiesAlongTime::BodyMap::const_iterator it = bm.begin ();
	 it != bm.end(); ++it)
	ostr << *(it->second) << endl;
    return ostr;
}


// BodyAlongTime Methods
// ======================================================================
void BodyAlongTime::CalculateBodyCenterWraps ()
{
    using G3D::Vector3;
    for (size_t time = 0; time < m_bodyAlongTime.size () - 1; time++)
    {
	const Vector3& begin = m_bodyAlongTime[time]->GetCenter ();
	const Vector3& end = m_bodyAlongTime[time + 1]->GetCenter ();
	m_centerMovement[time] = (end - begin).length ();
    }
    calculateStatisticsCenterMovement ();
    for (size_t time = 0; time < m_centerMovement.size (); time++)
    {
	if (fabs (m_centerMovement[time] - m_meanCenterMovement) > 
	    3 * m_stddevCenterMovement)
	    m_wraps.push_back (time);
    }
    m_wraps.push_back (m_centerMovement.size ());
}

void BodyAlongTime::calculateStatisticsCenterMovement ()
{
    m_meanCenterMovement = accumulate (
	m_centerMovement.begin (), m_centerMovement.end (), 0.0);
    m_meanCenterMovement /= m_centerMovement.size ();

    m_stddevCenterMovement = accumulate (
	m_centerMovement.begin (), m_centerMovement.end (), 0.0,
	squareDeviation (m_meanCenterMovement));
    m_stddevCenterMovement = sqrt (
	m_stddevCenterMovement / m_centerMovement.size ());
}

void BodyAlongTime::Resize ()
{
    Bodies::iterator it = find_if (
	m_bodyAlongTime.begin (), m_bodyAlongTime.end (), isNull);
    size_t size = it - m_bodyAlongTime.begin ();
    m_bodyAlongTime.resize (size);
    m_centerMovement.resize (size - 1);
}

// Static and Friends BodyAlongTime Methods
// ======================================================================
ostream& operator<< (
    ostream& ostr, const BodyAlongTime& bat)
{
    const BodyAlongTime::Bodies& bodies = bat.GetBodies ();
    ostr << "BodyAlongTime " << bodies[0]->GetOriginalIndex () 
	 << ": " << endl;
    ostr << "Center movement: ";
    ostream_iterator<float> of (ostr, " ");
    copy (bat.m_centerMovement.begin (), bat.m_centerMovement.end (), of);
    ostr << endl;

    ostr << "Center movement (mean, stddev) = (" 
	 << bat.m_meanCenterMovement << ", " 
	 << bat.m_stddevCenterMovement << ")" << endl;
    
    ostr << "Wraps: ";
    ostream_iterator<size_t> os (ostr, " ");
    copy (bat.m_wraps.begin (), bat.m_wraps.end (), os);
    ostr << endl;
    return ostr;
}
