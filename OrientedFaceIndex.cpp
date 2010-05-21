/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 20 May 2010
 *
 * Implementation of the OrientedFaceIndex class
 */

#include "OrientedFaceIndex.h"
#include "OrientedFace.h"

ostream& operator<< (ostream& ostr, const OrientedFaceIndex& ofi)
{
    OrientedFace of (ofi.m_face, ofi.m_faceReversed);
    ostr << "(oface=" << of.GetSignedIdString () << ", " 
	 << "index=" << ofi.m_edgeIndex << ")";
    return ostr;
}
