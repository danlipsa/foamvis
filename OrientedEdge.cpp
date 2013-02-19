/**
 * @file   OrientedFace.h
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */
#include "AttributeInfo.h"
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

const AdjacentOrientedFaces& OrientedEdge::GetAdjacentOrientedFaces () const
{
    return GetEdge ()->GetAdjacentOrientedFaces ();
}

size_t OrientedEdge::GetAdjacentOrientedFacesSize () const
{
    return GetEdge ()->GetAdjacentOrientedFacesSize ();
}


bool OrientedEdge::HasConstraints () const
{
    return GetEdge ()->HasConstraints ();
}

void OrientedEdge::AddAdjacentOrientedFace (
    boost::shared_ptr<OrientedFace> of, size_t edgeIndex) const
{
    GetEdge ()->AddAdjacentOrientedFace (of, edgeIndex);
}

string OrientedEdge::ToStringShort () const
{
    ostringstream ostr;
    ostr << "Oriented Edge " << GetStringId () << " "
	 << GetEdge ()->GetDuplicateStatus ()
	 << ": ";
    const Vertex& begin = GetBegin ();
    const Vertex& end = GetEnd ();
    ostr << begin.GetId () << "," << end.GetId ();    
    return ostr.str ();
}

string OrientedEdge::ToString (const AttributesInfo* ai) const
{
    ostringstream ostr;
    using G3D::Vector3;
    const AdjacentOrientedFaces& afs = GetAdjacentOrientedFaces ();
    ostr << "Oriented Edge " << GetStringId () << " "
	 << GetEdge ()->GetDuplicateStatus ()
	 << ": " << endl;
    const Vertex& begin = GetBegin ();
    const Vertex& end = GetEnd ();
    ostr << begin << "," << endl << end << endl;
    ostr << " Adjacent faces (" << afs.size () << "): ";
    BOOST_FOREACH (AdjacentOrientedFace af, afs)
	ostr << af << " ";
    if (GetEdge ()->HasAttributes ())
    {
	ostr << "Edge attributes: ";
	GetEdge ()->PrintAttributes (ostr, ai);
    }
    return ostr.str ();
}

void OrientedEdge::GetVertexSet (VertexSet* vertexSet) const
{
    GetEdge ()->GetVertexSet (vertexSet);
}

bool OrientedEdge::IsPhysical (bool is2D) const
{
    return GetEdge ()->IsPhysical (is2D);
}

size_t OrientedEdge::GetPointCount () const
{
    return GetEdge ()->GetPointCount ();
}

float OrientedEdge::GetLength () const
{
    return GetEdge ()->GetLength ();
}

G3D::Vector3 OrientedEdge::GetPoint (size_t i) const
{
    if (IsReversed ())
	i = GetPointCount () - 1 - i;
    return GetEdge ()->GetPoint (i);
}

size_t OrientedEdge::GetConstraintIndex () const
{
    return GetEdge ()->GetConstraintIndex (0);
}
