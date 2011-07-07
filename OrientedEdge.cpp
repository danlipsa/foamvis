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
    return boost::static_pointer_cast<Edge> (GetElement());
}

void OrientedEdge::SetEdge (boost::shared_ptr<Edge>  edge) 
{
    SetElement (edge);
}

G3D::Vector3 OrientedEdge::GetEdgeVector () const
{
    return GetEndVector () - GetBeginVector ();
}

boost::shared_ptr<Vertex> OrientedEdge::GetBeginPtr (void) const
{
    return IsReversed () ? GetEdge ()->GetEndPtr () : GetEdge ()->GetBeginPtr ();
}

G3D::Vector3 OrientedEdge::GetBeginVector () const
{
    return GetBegin ().GetVector ();
}


boost::shared_ptr<Vertex> OrientedEdge::GetEndPtr (void) const
{
    return IsReversed () ? GetEdge ()->GetBeginPtr () : GetEdge ()->GetEndPtr ();
}

G3D::Vector3 OrientedEdge::GetEndVector () const
{
    return GetEnd ().GetVector ();
}


bool OrientedEdge::IsZero () const
{
    return GetEdge ()->IsZero ();
}

const AdjacentOrientedFaces& OrientedEdge::GetAdjacentFaces () const
{
    return GetEdge ()->GetAdjacentFaces ();
}

void OrientedEdge::AddAdjacentFace (
    boost::shared_ptr<OrientedFace> of, size_t edgeIndex) const
{
    GetEdge ()->AddAdjacentFace (of, edgeIndex);
}

string OrientedEdge::ToString () const
{
    ostringstream ostr;
    using G3D::Vector3;
    ostr << "Oriented Edge " << GetStringId () << " "
	 << GetEdge ()->GetDuplicateStatus ()
	 << ": " << endl;
    const Vertex& begin = GetBegin ();
    const Vertex& end = GetEnd ();
    ostr << begin << "," << endl << end
	 << " Adjacent faces (" << GetAdjacentFaces ().size () << ")" << endl;
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
