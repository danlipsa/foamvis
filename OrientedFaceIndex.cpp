/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 20 May 2010
 *
 * Implementation of the OrientedFaceIndex class
 */

#include "OrientedFaceIndex.h"
#include "OrientedFace.h"
#include "Body.h"

ostream& operator<< (ostream& ostr, const OrientedFaceIndex& ofi)
{
    OrientedFace of (ofi.m_face, ofi.m_faceReversed);
    const BodyIndex& bi = of.GetBodyPartOf ();
    ostr << "(of=" << of.GetSignedIdString () << ", " 
	 << "oeI=" << ofi.m_orientedEdgeIndex 
	 << ", b=" << bi.m_body->GetId () 
	 << ", ofI=" << bi.m_orientedFaceIndex
	 << ")";
    return ostr;
}
