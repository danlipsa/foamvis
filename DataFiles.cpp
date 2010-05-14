/**
 * @file DataFiles.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Data objects
 */

#include "Body.h"
#include "Data.h"
#include "DataFiles.h"
#include "ElementUtils.h"


// Private Classes
// ======================================================================



// Private Functions
// ======================================================================
bool isNull (Body* body)
{
    return body == 0;
}


// Members
// ======================================================================
void DataFiles::Calculate (
    Aggregate aggregate, DataLessThanAlong::Corner corner, G3D::Vector3& v)
{
    using G3D::Vector3;
    vector<Data*>::iterator it;
    it = aggregate (m_data.begin (), m_data.end (), 
		    DataLessThanAlong(Vector3::X_AXIS, corner));
    v.x = ((*it)->*corner) ().x;

    it = aggregate (m_data.begin (), m_data.end (), 
	    DataLessThanAlong(Vector3::Y_AXIS, corner));    
    v.y = ((*it)->*corner) ().y;

    it = aggregate (m_data.begin (), m_data.end (), 
	    DataLessThanAlong(Vector3::Z_AXIS, corner));
    v.z = ((*it)->*corner) ().z;
}

void DataFiles::CalculateAABox ()
{
    using G3D::Vector3;
    Vector3 low, high;
    Calculate (min_element, &Data::GetAABoxLow, low);
    Calculate (max_element, &Data::GetAABoxHigh, high);
    m_AABox.set (low, high);
}

void DataFiles::CalculateWraps ()
{
    if (m_data.size () > 1 && m_data[0]->IsTorus ())
    {
    }
}

void DataFiles::CacheBodiesAlongTime ()
{
    for (size_t timeStep; timeStep < m_data.size (); timeStep++)
    {
	vector<Body*>& bodies = m_data[timeStep]->GetBodies ();
	BOOST_FOREACH (Body* body, bodies)
	    Body::CacheAlongTime (body->GetOriginalIndex (), timeStep, body);
    }
    pair<size_t, Body::BodyAlongTime> ibat;
    BOOST_FOREACH (ibat, Body::GetBodiesAlongTime ())
    {
	Body::BodyAlongTime& bat = ibat.second;
	Body::BodyAlongTime::iterator it = find_if (
	    bat.begin (), bat.end (), isNull);
	size_t size = it - bat.begin () + 1;
	Body::SetTimeSteps (size);
    }
}

// Static and Friend Members
//======================================================================

ostream& operator<< (ostream& ostr, DataFiles& dataFiles)
{
    ostr << "DataFiles: " << endl;
    ostr << dataFiles.m_AABox << endl;
    ostream_iterator<Data*> output (ostr, "\n");
    copy (dataFiles.m_data.begin (), dataFiles.m_data.end (), output);
    return ostr;
}
