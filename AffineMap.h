/**
 * @file AffineMap.h
 * @author Dan R. Lipsa
 * @date 17 May 2011
 *
 * Declaration of the AffineMap class
 */
#ifndef __AFFINE_MAP_H__
#define __AFFINE_MAP_H__

/**
 * Names of parameters in a DMP file where an affine transformation for a 
 * constraint is stored, and the constraint number.
 * Names for x, y and angle are stored.
 */
struct AffineMapNames
{
    string m_xName;
    string m_yName;
    string m_angleName;
    size_t m_constraint;
};

struct AffineMap
{
    G3D::Vector2 m_translation;
    float m_angle;
};

#endif //__AFFINE_MAP_H__

// Local Variables:
// mode: c++
// End:
