/**
 * @file   ElementUtils.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of various utility functions
 */

#include "ElementUtils.h"
#include "DebugStream.h"

ostream& operator<< (ostream& ostr,  G3D::AABox& box)
{
    cdbg << "AABox(" << box.low () << ", " << box.high () << ")";
    return ostr;
}

G3D::Matrix2 inverse(const G3D::Matrix2& o)
{
    const float* r0 = o[0];
    const float* r1 = o[1];
    return G3D::Matrix2(
	r1[1]/o.determinant (), -r0[1]/o.determinant (),
	-r1[0]/o.determinant (), r0[0]/o.determinant ());
}

G3D::Vector3 aNormal (const G3D::Vector3& v)
{
    G3D::Vector3 result (-v.y, v.x, 0);
    if (result.isZero ())
    {
	result.x = 0;
	result.y = - v.z;
	result.z = v.y;
    }
    return result;
}
