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
    return GetOrientedEdge ().IsReversed ();
}

OrientedEdge OrientedFaceIndex::GetOrientedEdge () const
{
    return GetOrientedFace ()->GetOrientedEdge (GetOrientedEdgeIndex ());
}

size_t OrientedFaceIndex::GetBodyId () const
{
    return GetBodyIndex ().GetBodyId ();
}

bool OrientedFaceIndex::IsValidNext (const OrientedFaceIndex& next) const
{
    using G3D::Vector3;
    Vector3 originalNormal = GetOrientedFace()->GetNormal ();
    Vector3 nextNormal = next.GetOrientedFace()->GetNormal ();
    Vector3 nextEdge = next.GetOrientedEdge ().GetEdgeVector ().unit ();
    Vector3 alongEdge = nextNormal.cross (originalNormal).direction ();
    return alongEdge.fuzzyEq (nextEdge);
}

boost::shared_ptr<Face> OrientedFaceIndex::GetFace () const
{
    return GetOrientedFace ()->GetFace ();
}

string OrientedFaceIndex::ToString () const
{
    ostringstream ostr;
    const BodyIndex& bi = GetBodyIndex ();
    ostr << "(" << bi
	 << ", of=" << GetOrientedFace ()->GetStringId () << ", " 
	 << "oeI=" << GetOrientedEdgeIndex ()
	 << ")";
    return ostr.str ();
}


// Static and friends methods
// ======================================================================
