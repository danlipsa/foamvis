/**
 * @file OrientedFace.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */

#include "OrientedFace.h"
#include "OrientedEdge.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Face.h"


void OrientedFace::GetOrientedEdge (
    size_t edgeIndex, OrientedEdge* oEdge) const
{
    Face::OrientedEdges& v = m_face->GetOrientedEdges ();
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
    *oEdge = OrientedEdge (oe->GetEdge (), reversed);
}

G3D::Vector3 OrientedFace::GetNormal () const
{
    G3D::Vector3 normal = m_face->GetNormal ();
    return m_reversed ? - normal : normal;
}

void OrientedFace::AddBodyPartOf (Body* body, size_t ofIndex) 
{
    m_face->AddBodyPartOf (body, ofIndex, m_reversed);
}

const BodyIndex& OrientedFace::GetBodyPartOf () const
{
    return m_face->GetBodyPartOf (m_reversed);
}

void OrientedFace::UpdateFacePartOf ()
{
    m_face->UpdateFacePartOf (m_reversed);
}

void OrientedFace::ClearFacePartOf ()
{
    m_face->ClearFacePartOf ();
}

void OrientedFace::ClearBodyPartOf ()
{
    m_face->ClearBodyPartOf ();
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

Vertex* OrientedFace::getBegin (size_t edgeIndex) const
{
    OrientedEdge oe;
    GetOrientedEdge (edgeIndex, &oe);
    return oe.GetBegin ();
}

Vertex* OrientedFace::getEnd (size_t edgeIndex) const
{
    OrientedEdge oe;
    GetOrientedEdge (edgeIndex, &oe);
    return oe.GetEnd ();
}

void OrientedFace::CalculateTranslation (
    const OrientedEdge& destination, size_t edgeIndex, 
    G3D::Vector3* translation) const
{
    OrientedEdge source;
    GetOrientedEdge (edgeIndex, &source);
    destination.CalculateTranslation (source, translation);
}

size_t OrientedFace::GetId () const
{
    return GetFace ()->GetId ();
}

// Static and Friends methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    ostr << (of.m_reversed ? "(R) " : "(N) ")
	 << "Oriented Face " << of.GetFace ()-> GetId () 
	 << " " << of.GetFace ()->GetColor () << " "
	 << of.GetFace ()->GetStatus ()
	 << ": " << endl;
    ostr << of.size () << " edges part of the face:" << endl;
    for (size_t i = 0; i < of.size (); i++)
    {
	OrientedEdge oe;
	of.GetOrientedEdge (i, &oe);
	ostr << i << ": " << oe << endl;
    }
    ostr << " Face attributes: ";
    return of.m_face->PrintAttributes (ostr);
}
