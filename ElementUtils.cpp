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
    using namespace G3D;
    cdbg << "AABox(" << box.low () << ", " << box.high () << ")";
    return ostr;
}
