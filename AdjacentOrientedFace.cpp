/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 * @date 20 May 2010
 *
 * Implementation of the AdjacentOrientedFace class
 */

#include "Body.h"
#include "AdjacentOrientedFace.h"
#include "OrientedFace.h"
#include "OrientedEdge.h"

const AdjacentBody& AdjacentOrientedFace::GetAdjacentBody () const
{
    return GetOrientedFace ()->GetAdjacentBody ();
}

bool AdjacentOrientedFace::IsOrientedEdgeReversed () const
{
    return GetOrientedEdge ().IsReversed ();
}

OrientedEdge AdjacentOrientedFace::GetOrientedEdge () const
{
    return GetOrientedFace ()->GetOrientedEdge (GetOrientedEdgeIndex ());
}

size_t AdjacentOrientedFace::GetBodyId () const
{
    return GetAdjacentBody ().GetBodyId ();
}

bool AdjacentOrientedFace::IsValidNext (const AdjacentOrientedFace& next) const
{
    using G3D::Vector3;
    Vector3 originalNormal = GetOrientedFace()->GetNormal ();
    Vector3 nextNormal = next.GetOrientedFace()->GetNormal ();
    Vector3 nextEdge = next.GetOrientedEdge ().GetEdgeVector ().unit ();
    Vector3 alongEdge = nextNormal.cross (originalNormal).direction ();
    return alongEdge.fuzzyEq (nextEdge);
}

boost::shared_ptr<Face> AdjacentOrientedFace::GetFace () const
{
    return GetOrientedFace ()->GetFace ();
}

string AdjacentOrientedFace::ToString () const
{
    ostringstream ostr;
    const AdjacentBody& bi = GetAdjacentBody ();
    ostr << "(" << bi
	 << ", of=" << GetOrientedFace ()->GetStringId () << ", " 
	 << "oeI=" << GetOrientedEdgeIndex ()
	 << ")";
    return ostr.str ();
}

bool AdjacentOrientedFace::IsStandalone () const
{
    return GetOrientedFace ()->IsStandalone ();
}

