/**
 * @file FoamAlongTime.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Foam objects
 */

#include "Body.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "Utils.h"

// Private Functions
// ======================================================================
void calculateBodyWraps (BodiesAlongTime::BodyMap::value_type& v,
    const FoamAlongTime& foamAlongTime)
{
    v.second->CalculateBodyWraps (foamAlongTime);
}


// Members
// ======================================================================
void FoamAlongTime::Calculate (
    Aggregate aggregate, FoamLessThanAlong::Corner corner, G3D::Vector3& v)
{
    using G3D::Vector3;
    Foams::iterator it;
    it = aggregate (m_foams.begin (), m_foams.end (), 
		    FoamLessThanAlong(Vector3::X_AXIS, corner));
    v.x = ((*it).get()->*corner) ().x;

    it = aggregate (m_foams.begin (), m_foams.end (), 
	    FoamLessThanAlong(Vector3::Y_AXIS, corner));    
    v.y = ((*it).get()->*corner) ().y;

    it = aggregate (m_foams.begin (), m_foams.end (), 
	    FoamLessThanAlong(Vector3::Z_AXIS, corner));
    v.z = ((*it).get()->*corner) ().z;
}

void FoamAlongTime::CalculateAABox ()
{
    using G3D::Vector3;
    Vector3 low, high;
    Calculate (min_element, &Foam::GetAABoxLow, low);
    Calculate (max_element, &Foam::GetAABoxHigh, high);
    m_AABox.set (low, high);
}

void FoamAlongTime::CalculateBodyWraps ()
{
    if (m_foams.size () > 1)
    {
	BodiesAlongTime::BodyMap bodyMap = GetBodiesAlongTime ().GetBodyMap ();
	for_each (bodyMap.begin (), bodyMap.end (),
		  boost::bind(calculateBodyWraps, _1, *this));
    }
}

void FoamAlongTime::PostProcess ()
{
    CalculateAABox ();
    CacheBodiesAlongTime ();
    CalculateBodyWraps ();
    GetBodiesAlongTime ().CalculateSpeedRange (*this);
}

void FoamAlongTime::CacheBodiesAlongTime ()
{
    Foam::Bodies& bodies = m_foams[0]->GetBodies ();
    for_each (bodies.begin (), bodies.end (), 
	      boost::bind (&BodiesAlongTime::Allocate,
			   &m_bodiesAlongTime, _1, m_foams.size ()));
    for (size_t timeStep = 0; timeStep < m_foams.size (); timeStep++)
    {
	Foam::Bodies& bodies = m_foams[timeStep]->GetBodies ();
	BOOST_FOREACH (boost::shared_ptr<Body>  body, bodies)
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
    ostream_iterator< boost::shared_ptr<const Foam> > output (ostr, "\n");
    copy (dataAlongTime.m_foams.begin (), dataAlongTime.m_foams.end (), output);
    ostr << dataAlongTime.m_bodiesAlongTime;
    return ostr;
}
