/**
 * @file   Utils.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of various utility functions
 */

#include "Utils.h"
#include "DebugStream.h"

const G3D::Vector3int16 Vector3int16Zero (0, 0, 0);

ostream& operator<< (ostream& ostr, const G3D::AABox& box)
{
    cdbg << "AABox(" << box.low () << ", " << box.high () << ")";
    return ostr;
}


const G3D::Vector3int16& Vector3int16Unit (size_t direction)
{
    static const G3D::Vector3int16 unitVector3int16[3] = {
	G3D::Vector3int16 (1, 0, 0),
	G3D::Vector3int16 (0, 1, 0),
	G3D::Vector3int16 (0, 0, 1)
    };
    return unitVector3int16[direction];
}
