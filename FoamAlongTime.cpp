/**
 * @file FoamAlongTime.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Foam objects
 */

#include "Body.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "ElementUtils.h"

// Private Functions
// ======================================================================
void calculateWraps (BodiesAlongTime::BodyMap::value_type& v)
{
    v.second->CalculateBodyCenterWraps ();
}


// Members
// ======================================================================
void FoamAlongTime::Calculate (
    Aggregate aggregate, FoamLessThanAlong::Corner corner, G3D::Vector3& v)
{
    using G3D::Vector3;
    vector<Foam*>::iterator it;
    it = aggregate (m_data.begin (), m_data.end (), 
		    FoamLessThanAlong(Vector3::X_AXIS, corner));
    v.x = ((*it)->*corner) ().x;

    it = aggregate (m_data.begin (), m_data.end (), 
	    FoamLessThanAlong(Vector3::Y_AXIS, corner));    
    v.y = ((*it)->*corner) ().y;

    it = aggregate (m_data.begin (), m_data.end (), 
	    FoamLessThanAlong(Vector3::Z_AXIS, corner));
    v.z = ((*it)->*corner) ().z;
}

void FoamAlongTime::CalculateAABox ()
{
    using G3D::Vector3;
    Vector3 low, high;
    Calculate (min_element, &Foam::GetAABoxLow, low);
    Calculate (max_element, &Foam::GetAABoxHigh, high);
    m_AABox.set (low, high);
}

void FoamAlongTime::CalculateBodyCenterWraps ()
{
    if (m_data.size () > 1 && m_data[0]->IsTorus ())
    {
	BodiesAlongTime::BodyMap bodyMap = m_bodiesAlongTime.GetBodyMap ();
	for_each (bodyMap.begin (), bodyMap.end (), calculateWraps);
    }
}

void FoamAlongTime::PostProcess ()
{
    CalculateAABox ();
    CacheBodiesAlongTime ();
    CalculateBodyCenterWraps ();
}

void FoamAlongTime::CacheBodiesAlongTime ()
{
    if (m_data.size () <= 1)
	return;
    Foam::Bodies& bodies = m_data[0]->GetBodies ();
    for_each (bodies.begin (), bodies.end (), 
	      boost::bind (&BodiesAlongTime::Allocate,
			   &m_bodiesAlongTime, _1, m_data.size ()));
    for (size_t timeStep = 0; timeStep < m_data.size (); timeStep++)
    {
	Foam::Bodies& bodies = m_data[timeStep]->GetBodies ();
	BOOST_FOREACH (Body* body, bodies)
	    m_bodiesAlongTime.Cache (body, timeStep);
    }
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&BodiesAlongTime::Resize, &m_bodiesAlongTime, _1));
}

// Static and Friend Members
//======================================================================

ostream& operator<< (ostream& ostr, const FoamAlongTime& dataAlongTime)
{
    ostr << "FoamAlongTime: " << endl;
    ostr << dataAlongTime.m_AABox << endl;
    ostream_iterator<Foam*> output (ostr, "\n");
    copy (dataAlongTime.m_data.begin (), dataAlongTime.m_data.end (), output);
    ostr << dataAlongTime.m_bodiesAlongTime;
    return ostr;
}
