/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 20 May 2010
 *
 * Implementation of the OrientedFaceIndex class
 */

#include "Body.h"
#include "OrientedFaceIndex.h"
#include "OrientedFace.h"
#include "OrientedEdge.h"

const BodyIndex& OrientedFaceIndex::GetBodyIndex () const
{
    return GetOrientedFace ()->GetBodyPartOf ();
}

bool OrientedFaceIndex::IsOrientedEdgeReversed () const
{
    OrientedEdge orientedEdge;
    GetOrientedEdge (&orientedEdge);
    return orientedEdge.IsReversed ();
}

void OrientedFaceIndex::GetOrientedEdge (OrientedEdge* oe) const
{
    GetOrientedFace ()->GetOrientedEdge (GetOrientedEdgeIndex (), oe);
}

size_t OrientedFaceIndex::GetBodyId () const
{
    return GetBodyIndex ().GetBodyId ();
}


// Static and friends methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OrientedFaceIndex& ofi)
{
    ostr << "(of=" << ofi.GetOrientedFace ()->GetSignedIdString () << ", " 
	 << "oeI=" << ofi.GetOrientedEdgeIndex ()
	 << ")";
    return ostr;
}
