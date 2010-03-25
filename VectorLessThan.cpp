/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 25 March 2010
 *
 * Implementation comparison functors between vectors
 */

#include "VectorLessThan.h"


bool VectorLessThanAngle::operator () (
    const G3D::Vector3& first, const G3D::Vector3& second) const
{
    using G3D::Vector3;
    double firstAngle = angle0pi (first, m_originNormal);
    double secondAngle = angle0pi (second, m_originNormal);
    if (firstAngle < secondAngle)
	return true;
    else if (firstAngle == secondAngle)
    {
	Vector3 normal1, normal2;
	m_originNormal.getTangents (normal1, normal2);
	if (angle (first, normal1) < angle (second, normal1))
	    return true;
	else
	    return false;
    }
    else
	return false;
}

double VectorLessThanAngle::angle (
    const G3D::Vector3& first, const G3D::Vector3& second)
{
    double angle = acos (first.dot (second));
    double sinValue = first.cross (second).length ();
    if (sinValue < 0)
	angle = 2*M_PI - angle;
    return angle;
}
