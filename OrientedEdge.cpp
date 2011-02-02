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
#include "Utils.h"
#include "Vertex.h"

OrientedEdge::OrientedEdge (
    const boost::shared_ptr<Edge>& edge, bool reversed) : 
    OrientedElement (edge, reversed)
{
}

boost::shared_ptr<Edge> OrientedEdge::GetEdge () const 
{
    return boost::static_pointer_cast<Edge> (GetColoredElement());
}

void OrientedEdge::SetEdge (boost::shared_ptr<Edge>  edge) 
{
    SetElement (edge);
}

ostream& OrientedEdge::print (ostream& ostr, bool reversed) const
{
    using G3D::Vector3;
    ostr << "Oriented Edge " << GetStringId () << " "
	 << GetEdge ()->GetDuplicateStatus ()
	 << ": ";
    const Vector3* begin = &GetBegin ()->GetVector ();
    const Vector3* end = &GetEnd ()->GetVector ();;
    if (reversed)
	swap (begin, end);
    ostr << *begin << ", " << *end;
    return ostr;
}

G3D::Vector3 OrientedEdge::GetEdgeVector () const
{
    return GetEnd ()->GetVector () - GetBegin ()->GetVector ();
}

boost::shared_ptr<Vertex> OrientedEdge::GetBegin (void) const
{
    return IsReversed () ? GetEdge ()->GetEnd () : GetEdge ()->GetBegin ();
}

boost::shared_ptr<Vertex> OrientedEdge::GetEnd (void) const
{
    return IsReversed () ? GetEdge ()->GetBegin () : GetEdge ()->GetEnd ();
}

bool OrientedEdge::IsZero () const
{
    return GetEdge ()->IsZero ();
}

size_t OrientedEdge::GetFacePartOfSize () const
{
    return GetEdge ()->GetFacePartOfSize ();
}

OrientedFaceIndexList::const_iterator OrientedEdge::GetFacePartOfBegin () const
{
    return GetEdge ()->GetFacePartOfBegin ();
}

OrientedFaceIndexList::const_iterator OrientedEdge::GetFacePartOfEnd () const
{
    return GetEdge ()->GetFacePartOfEnd ();
}

void OrientedEdge::AddFacePartOf (
    boost::shared_ptr<OrientedFace> of, size_t edgeIndex) const
{
    GetEdge ()->AddFacePartOf (of, edgeIndex);
}

string OrientedEdge::ToString () const
{
    ostringstream ostr;
    print (ostr);
    return ostr.str ();
}

void OrientedEdge::GetVertexSet (VertexSet* vertexSet) const
{
    GetEdge ()->GetVertexSet (vertexSet);
}

bool OrientedEdge::IsPhysical (size_t dimension, bool isQuadratic) const
{
    return GetEdge ()->IsPhysical (dimension, isQuadratic);
}

size_t OrientedEdge::PointCount () const
{
    return GetEdge ()->PointCount ();
}

G3D::Vector3 OrientedEdge::GetPoint (size_t i) const
{
    if (IsReversed ())
	i = PointCount () - 1 - i;
    return GetEdge ()->GetPoint (i);
}
