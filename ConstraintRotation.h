/**
 * @file ConstraintRotation.h
 * @author Dan R. Lipsa
 * @date 17 May 2011
 *
 * Declaration of the ConstraintRotation class
 */
#ifndef __CONSTRAINT_ROTATION_H__
#define __CONSTRAINT_ROTATION_H__

#include "Utils.h"

/**
 * Names of parameters in a DMP file where a rotation for a 
 * constraint is stored, and the constraint number.
 * Names for x, y and angle are stored.
 */
struct ConstraintRotationNames
{
    ConstraintRotationNames () :
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

struct ConstraintRotation
{
    ConstraintRotation () :
	m_angle (0)
    {
    }
    G3D::Vector2 m_center;
    float m_angle;
};

#endif //__CONSTRAINT_ROTATION_H__

// Local Variables:
// mode: c++
// End:
