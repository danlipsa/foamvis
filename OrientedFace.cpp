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


OrientedFace::OrientedFace(Face* face, bool reversed) : 
    OrientedElement (face, reversed) 
{
}

Face* OrientedFace::GetFace () const
{
    return static_cast<Face*>(GetElement());
}

void OrientedFace::SetFace (Face* face)
{
    SetElement (face);
}

void OrientedFace::GetOrientedEdge (
    size_t edgeIndex, OrientedEdge* oEdge) const
{
    Face::OrientedEdges& v = GetFace ()->GetOrientedEdges ();
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
    G3D::Vector3 normal = GetFace ()->GetNormal ();
    return IsReversed () ? - normal : normal;
}

void OrientedFace::AddBodyPartOf (Body* body, size_t ofIndex) 
{
    GetFace ()->AddBodyPartOf (body, ofIndex, IsReversed ());
}

const BodyIndex& OrientedFace::GetBodyPartOf () const
{
    return GetFace ()->GetBodyPartOf (IsReversed ());
}

void OrientedFace::UpdateFacePartOf ()
{
    GetFace ()->UpdateFacePartOf (IsReversed ());
}

void OrientedFace::ClearFacePartOf ()
{
    GetFace ()->ClearFacePartOf ();
}

void OrientedFace::ClearBodyPartOf ()
{
    GetFace ()->ClearBodyPartOf ();
}

size_t OrientedFace::GetNextValidIndex (size_t index) const
{
    return GetFace ()->GetNextValidIndex (index);
}

size_t OrientedFace::GetPreviousValidIndex (size_t index) const
{
    return GetFace ()->GetPreviousValidIndex (index);
}

size_t OrientedFace::size () const
{
    return GetFace ()->GetEdgeCount ();
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

bool OrientedFace::IsPartOfBody (size_t bodyId) const
{
    return GetFace ()->IsPartOfBody (bodyId, IsReversed ());
}


// Static and Friends methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    ostr << "Oriented Face " << of.GetSignedIdString () 
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
    return of.GetFace ()->PrintAttributes (ostr);
}

