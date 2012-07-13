/**
 * @file ObjectPosition.h
 * @author Dan R. Lipsa
 * @date 17 May 2011
 *
 * Declaration of the ObjectPosition class
 */
#ifndef __OBJECT_POSITION_H__
#define __OBJECT_POSITION_H__

#include "Utils.h"

/**
 * Names of parameters in a DMP file where a rotation for a 
 * constraint is stored, and the constraint number.
 * Names for x, y and angle are stored.
 */
struct DmpObjectInfo
{
    DmpObjectInfo () :
	m_constraintIndex (INVALID_INDEX)
    {
    }
    bool ConstraintUsed () const
    {
	return m_constraintIndex != INVALID_INDEX;
    }
    bool RotationUsed () const
    {
	return ! m_xName.empty ();
    }
    size_t m_constraintIndex;
    string m_xName;
    string m_yName;
    string m_angleName;
};

struct ObjectPosition
{
    ObjectPosition () :
	m_angleRadians (0)
    {
    }
    G3D::Vector3 m_rotationCenter;
    float m_angleRadians;
};

#endif //__OBJECT_POSITION_H__

// Local Variables:
// mode: c++
// End:
