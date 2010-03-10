/**
 * @file DataFiles.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Data objects
 */

#include "DataFiles.h"
#include "ElementUtils.h"

ostream& operator<< (ostream& ostr, DataFiles& dataFiles)
{
    ostr << "DataFiles: " << endl;
    ostr << dataFiles.m_AABox << endl;
    PrintElements<Data*> (ostr, dataFiles.m_data, "Data", true);
    return ostr;
}

void DataFiles::Calculate (
    Aggregate aggregate, Data::Corner corner, G3D::Vector3& v)
{
    using G3D::Vector3;
    vector<Data*>::iterator it;
    it = aggregate (m_data.begin (), m_data.end (), 
		    Data::LessThanAlong(Vector3::X_AXIS, corner));
    v.x = ((*it)->*corner) ().x;

    it = aggregate (m_data.begin (), m_data.end (), 
	    Data::LessThanAlong(Vector3::Y_AXIS, corner));    
    v.y = ((*it)->*corner) ().y;

    it = aggregate (m_data.begin (), m_data.end (), 
	    Data::LessThanAlong(Vector3::Z_AXIS, corner));
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
