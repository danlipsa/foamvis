/**
 * @file OrientedFace.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */

#include "OrientedFace.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Face.h"

ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    ostr << (of.m_reversed ? "(R) " : "(N) ")
	 << "Oriented Face " << of.GetFace ()-> GetOriginalIndex () 
	 << " " << of.GetFace ()->GetColor () << " "
	 << (of.GetFace ()->IsDuplicate () ? " DUPLICATE" : "")
	 << ": " << endl;
    ostr << of.size () << " edges part of the face:" << endl;
    for (size_t i = 0; i < of.size (); i++)
    {
	const OrientedEdge oe = of.GetOrientedEdge (i);
	ostr << i << ": " << oe << endl;
    }
    ostr << " Face attributes: ";
    return of.m_face->PrintAttributes (ostr);
}

OrientedEdge OrientedFace::GetOrientedEdge (size_t edgeIndex) const
{
    vector<OrientedEdge*>& v = m_face->GetOrientedEdges ();
    RuntimeAssert (edgeIndex < v.size (),
		   "Edge index ", edgeIndex,
		   " greater than the number of edges ", v.size ());
    OrientedEdge* oe;
    if (IsReversed ())
    {
        oe = v[v.size() - 1 - edgeIndex];
    }
    else
    {
        oe = v[edgeIndex];
    }
    bool reversed = oe->IsReversed ();
    if (IsReversed ())
	reversed = ! reversed;
    return OrientedEdge (oe->GetEdge (), reversed);
}

G3D::Vector3 OrientedFace::GetNormal () const
{
    G3D::Vector3 normal = m_face->GetNormal ();
    return m_reversed ? - normal : normal;
}

void OrientedFace::AddAdjacentBody (Body* body) 
{
    m_face->AddAdjacentBody (body);
}

vector<Body*>& OrientedFace::GetAdjacentBodies ()
{
    return m_face->GetAdjacentBodies ();
}

void OrientedFace::UpdateEdgesAdjacency ()
{
    m_face->UpdateEdgesAdjacency ();
}

void OrientedFace::ClearEdgesAdjacency ()
{
    m_face->ClearEdgesAdjacency ();
}

size_t OrientedFace::GetNextValidIndex (size_t index) const
{
    return m_face->GetNextValidIndex (index);
}

size_t OrientedFace::GetPreviousValidIndex (size_t index) const
{
    return m_face->GetPreviousValidIndex (index);
}

size_t OrientedFace::size () const
{
    return m_face->GetEdgeCount ();
}
