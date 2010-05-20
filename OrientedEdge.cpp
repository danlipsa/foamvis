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
    return m_reversed ? m_edge->GetEnd () : m_edge->GetBegin ();
}

Vertex* OrientedEdge::GetEnd (void) const
{
    return m_reversed ? m_edge->GetBegin () : m_edge->GetEnd ();
}


void OrientedEdge::ClearFacePartOf ()
{
    m_edge->ClearFacePartOf ();
}

bool OrientedEdge::IsZero () const
{
    return m_edge->IsZero ();
}

size_t OrientedEdge::GetFacePartOfSize () const
{
    return m_edge->GetFacePartOfSize ();
}

const OrientedFaceIndex& OrientedEdge::GetFacePartOf (size_t i) const
{
    return m_edge->GetFacePartOf (m_reversed, i);
}

void OrientedEdge::AddFacePartOf (
    Face* face, bool faceReversed, size_t edgeIndex)
{
    m_edge->AddFacePartOf (face, faceReversed, edgeIndex, m_reversed);
}

size_t OrientedEdge::GetId () const
{
    return m_edge->GetId ();
}
