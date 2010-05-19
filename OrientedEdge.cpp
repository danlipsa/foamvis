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

ostream& OrientedEdge::print (ostream& ostr, bool reversed) const
{
    using G3D::Vector3;
    ostr << (m_reversed ? "(R)" : "(N)");
    ostr << "Oriented Edge " << GetEdge ()-> GetId () << " "
	 << GetEdge ()->GetStatus ()
	 << ": ";
    const Vector3* begin = static_cast<const G3D::Vector3*>(GetBegin ());
    const Vector3* end = static_cast<const G3D::Vector3*>(GetEnd ());
    if (reversed)
	swap (begin, end);
    ostr << *begin << ", " << *end;
    return ostr;
}

bool OrientedEdge::Fits (const OrientedEdge& destination,
			 G3D::Vector3* translation) const
{
    if (GetEdge ()->GetId () == 
	destination.GetEdge ()->GetId () &&
	IsReversed () == ! destination.IsReversed ())
    {
	if (translation != 0)
	    *translation = *destination.GetEnd () - *GetBegin ();
	return true;
    }
    else
	return false;
}

G3D::Vector3 OrientedEdge::GetEdgeVector () const
{
    return *GetEnd () - *GetBegin ();
}

Vertex* OrientedEdge::GetBegin (void) const
{
    return m_reversed ? m_edge->GetEnd () : m_edge->GetBegin ();
}

Vertex* OrientedEdge::GetEnd (void) const
{
    return m_reversed ? m_edge->GetBegin () : m_edge->GetEnd ();
}


void OrientedEdge::ClearAdjacentFaces ()
{
    m_edge->ClearAdjacentFaces ();
}

bool OrientedEdge::IsZero () const
{
    return m_edge->IsZero ();
}


void OrientedEdge::GetAdjacentOrientedFaceIndex (
    size_t faceIndex, OrientedFace* ofResult, size_t* indexResult) const
{
    OrientedFaceIndex ofi =
	m_edge->GetAdjacentOrientedFaceIndex (faceIndex);
    OrientedFace& of = *ofi.m_orientedFace;
    size_t edgeIndex = ofi.m_index;
    size_t reversed = of.IsReversed ();
    if (m_reversed)
    {
	*indexResult = of.size () - edgeIndex - 1;
	reversed = ! reversed;
    }
    else
	*indexResult = edgeIndex;
    *ofResult = OrientedFace (of.GetFace (), reversed);
}

void OrientedEdge::AddAdjacentFace (Face* face, size_t oEdgeIndex)
{
    size_t edgeIndex;
    if (m_reversed)
    {
	edgeIndex = face->size () - 1 - oEdgeIndex;
    }
    else
	edgeIndex = oEdgeIndex;
    m_edge->AddAdjacentOrientedFace (
	OrientedFace (face, m_reversed), edgeIndex);
}
