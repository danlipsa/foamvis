/**
 * @file   OrientedFace.h
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */
#include "Debug.h"
#include "Edge.h"
#include "Face.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "Vertex.h"

OrientedEdge::OrientedEdge (Edge* edge, bool reversed) : 
    OrientedElement (edge, reversed) 
{
}

Edge* OrientedEdge::GetEdge () const 
{
    return static_cast<Edge*>(GetElement());
}

void OrientedEdge::SetEdge (Edge* edge) 
{
    SetElement (edge);
}


ostream& OrientedEdge::print (ostream& ostr, bool reversed) const
{
    using G3D::Vector3;
    ostr << "Oriented Edge " << GetSignedIdString () << " "
	 << GetEdge ()->GetStatus ()
	 << ": ";
    const Vector3* begin = static_cast<const G3D::Vector3*>(GetBegin ());
    const Vector3* end = static_cast<const G3D::Vector3*>(GetEnd ());
    if (reversed)
	swap (begin, end);
    ostr << *begin << ", " << *end;
    return ostr;
}

void OrientedEdge::CalculateTranslation (
    const OrientedEdge& source, G3D::Vector3* translation) const
{
    *translation = *GetBegin () - *source.GetEnd ();
}

G3D::Vector3 OrientedEdge::GetEdgeVector () const
{
    return *GetEnd () - *GetBegin ();
}

Vertex* OrientedEdge::GetBegin (void) const
{
    return IsReversed () ? GetEdge ()->GetEnd () : GetEdge ()->GetBegin ();
}

Vertex* OrientedEdge::GetEnd (void) const
{
    return IsReversed () ? GetEdge ()->GetBegin () : GetEdge ()->GetEnd ();
}


void OrientedEdge::ClearFacePartOf ()
{
    GetEdge ()->ClearFacePartOf ();
}

bool OrientedEdge::IsZero () const
{
    return GetEdge ()->IsZero ();
}

size_t OrientedEdge::GetFacePartOfSize () const
{
    return GetEdge ()->GetFacePartOfSize ();
}

const OrientedFaceIndex& OrientedEdge::GetFacePartOf (size_t i) const
{
    return GetEdge ()->GetFacePartOf (i);
}

void OrientedEdge::AddFacePartOf (OrientedFace* face, size_t edgeIndex) const
{
    GetEdge ()->AddFacePartOf (face, edgeIndex);
}

