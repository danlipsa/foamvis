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


OrientedFace::OrientedFace(const boost::shared_ptr<Face>& face, bool reversed) : 
    OrientedElement (face, reversed) 
{
}

boost::shared_ptr<Face>  OrientedFace::GetFace () const
{
    return boost::static_pointer_cast<Face>(GetColoredElement());
}

void OrientedFace::SetFace (boost::shared_ptr<Face>  face)
{
    SetElement (face);
}

OrientedEdge OrientedFace::GetOrientedEdge (
    size_t edgeIndex) const
{
    Face::OrientedEdges& v = GetFace ()->GetOrientedEdges ();
    RuntimeAssert (edgeIndex < v.size (),
		   "Edge index ", edgeIndex,
		   " greater than the number of edges ", v.size ());
    boost::shared_ptr<OrientedEdge> oe;
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
    G3D::Vector3 normal = GetFace ()->GetNormal ();
    return IsReversed () ? - normal : normal;
}

void OrientedFace::AddBodyPartOf (boost::shared_ptr<Body>  body, size_t ofIndex) 
{
    GetFace ()->AddBodyPartOf (body, ofIndex);
}

const BodyIndex& OrientedFace::GetBodyPartOf () const
{
    return GetFace ()->GetBodyPartOf (IsReversed ());
}

void OrientedFace::UpdateFacePartOf (boost::shared_ptr<OrientedFace> of)
{
    for (size_t i = 0; i < size (); i++)
    {
	const OrientedEdge& oe = GetOrientedEdge (i);
	oe.AddFacePartOf (of, i);
    }
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

boost::shared_ptr<Vertex> OrientedFace::getBegin (size_t edgeIndex) const
{
    const OrientedEdge& oe = GetOrientedEdge (edgeIndex);
    return oe.GetBegin ();
}

boost::shared_ptr<Vertex> OrientedFace::getEnd (size_t edgeIndex) const
{
    const OrientedEdge& oe = GetOrientedEdge (edgeIndex);
    return oe.GetEnd ();
}

ostream& OrientedFace::PrintAttributes (ostream& ostr) const
{
    return GetFace ()->PrintAttributes (ostr);
}


// Static and Friends methods
// ======================================================================

ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    ostr << "Oriented Face " << of.GetStringId () 
	 << " " << of.GetFace ()->GetColor () << " "
	 << of.GetFace ()->GetDuplicateStatus ()
	 << ": " << endl;
    ostr << of.size () << " edges part of the face:" << endl;
    for (size_t i = 0; i < of.size (); i++)
	ostr << i << ": " << of.GetOrientedEdge (i) << endl;
    ostr << " Face attributes: ";
    return of.GetFace ()->PrintAttributes (ostr);
}

