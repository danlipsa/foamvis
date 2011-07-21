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

boost::shared_ptr<Body> AdjacentOrientedFace::GetBody () const
{
    return GetAdjacentBody ().GetBody ();
}

bool AdjacentOrientedFace::IsValidNext (const AdjacentOrientedFace& next) const
{
    using G3D::Vector3;
    Vector3 thisNormal = GetOrientedFace()->GetNormal ();
    Vector3 nextNormal = next.GetOrientedFace()->GetNormal ();
    Vector3 nextEdge = next.GetOrientedEdge ().GetEdgeVector ().unit ();
    Vector3 alongEdge = nextNormal.cross (thisNormal).direction ();
    return alongEdge.fuzzyEq (nextEdge);
}

boost::shared_ptr<Face> AdjacentOrientedFace::GetFace () const
{
    return GetOrientedFace ()->GetFace ();
}

string AdjacentOrientedFace::ToString () const
{
    boost::shared_ptr<OrientedFace> of = GetOrientedFace ();
    ostringstream ostr;
    if (of->IsStandalone ())
	ostr << "(standalone face";
    else
    {
	const AdjacentBody& bi = GetAdjacentBody ();
	ostr << "(" << bi;
    }
    ostr << ", of=" << of->GetStringId () << ", " 
	 << "oeI=" << GetOrientedEdgeIndex ()
	 << ")";
    return ostr.str ();
}

bool AdjacentOrientedFace::IsStandalone () const
{
    return GetOrientedFace ()->IsStandalone ();
}

