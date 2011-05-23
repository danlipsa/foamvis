/**
 * @file ConstraintRotation.h
 * @author Dan R. Lipsa
 * @date 17 May 2011
 *
 * Declaration of the ConstraintRotation class
 */
#ifndef __CONSTRAINT_ROTATION_H__
#define __CONSTRAINT_ROTATION_H__

/**
 * Names of parameters in a DMP file where a rotation for a 
 * constraint is stored, and the constraint number.
 * Names for x, y and angle are stored.
 */
struct ConstraintRotationNames
{
    string m_xName;
    string m_yName;
    string m_angleName;
    size_t m_constraintIndex;
};

struct ConstraintRotation
{
    G3D::Vector2 m_center;
    float m_angle;
};

#endif //__CONSTRAINT_ROTATION_H__

// Local Variables:
// mode: c++
// End:
