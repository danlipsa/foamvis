/**
 * @file   OOBox.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the OOBox class
 */

#include "OOBox.h"
#include "Debug.h"

const G3D::Vector3& OOBox::operator[] (size_t i) const
{
    switch (i)
    {
    case 0:
	return m_x;
    case 1:
	return m_y;
    case 2:
	return m_z;
    default:
	RuntimeAssert (false, "Invalid index for OOBox");
	return m_x;
    }
}

G3D::Vector3 OOBox::Translate (const G3D::Vector3& v,
				const G3D::Vector3int16& domainIncrement) const
{
    G3D::Vector3 ret = v;
    for (int i = 0; i < 3; i++)
	ret += (*this)[i] * domainIncrement[i];
    return ret;
}


// Static and friends Methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OOBox& box)
{
    return ostr << box.m_x << endl << box.m_y << endl << box.m_z << endl;
}
