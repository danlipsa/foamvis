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
    using G3D::Vector3;
    ostr << "Oriented Edge " << GetStringId () << " "
	 << GetEdge ()->GetDuplicateStatus ()
	 << ": " << endl;
    boost::shared_ptr<Vertex> begin = GetBegin ();
    boost::shared_ptr<Vertex> end = GetEnd ();
    if (IsReversed ())
	swap (begin, end);
    ostr << *begin << "," << endl << *end << endl;
    if (GetEdge ()->HasAttributes ())
    {
	ostr << "Edge attributes: ";
	GetEdge ()->PrintAttributes (ostr);
    }
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

size_t OrientedEdge::GetPointCount () const
{
    return GetEdge ()->GetPointCount ();
}

double OrientedEdge::GetLength () const
{
    return GetEdge ()->GetLength ();
}

G3D::Vector3 OrientedEdge::GetPoint (size_t i) const
{
    if (IsReversed ())
	i = GetPointCount () - 1 - i;
    return GetEdge ()->GetPoint (i);
}
