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

boost::shared_ptr<Face> OrientedFace::GetFace () const
{
    return boost::static_pointer_cast<Face>(GetColoredElement());
}

void OrientedFace::SetFace (const boost::shared_ptr<Face>& face)
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

void OrientedFace::AddBodyPartOf (const boost::shared_ptr<Body>& body, 
				  size_t ofIndex) 
{
    GetFace ()->AddBodyPartOf (body, ofIndex);
}

const BodyIndex& OrientedFace::GetBodyPartOf () const
{
    return GetFace ()->GetBodyPartOf (IsReversed ());
}

void OrientedFace::UpdateFacePartOf (const boost::shared_ptr<OrientedFace>& of)
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

bool OrientedFace::IsTriangle () const
{
    return GetFace ()->IsTriangle ();
}

const G3D::Vector3& OrientedFace::GetCenter () const
{
    return GetFace ()->GetCenter ();
}

bool OrientedFace::IsClosed () const
{
    return GetFace ()->IsClosed ();
}

double OrientedFace::GetPerimeter () const
{
    return GetFace ()->GetPerimeter ();
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

string OrientedFace::ToString () const
{
    ostringstream ostr;
    boost::shared_ptr<Face> face = GetFace ();
    ostr << "Oriented Face " << GetStringId () << " " 
	 << face->GetDuplicateStatus () << " useCount=" << face.use_count ()
	 << ": " << endl;
    ostr << size () << " edges part of the face:" << endl;
    for (size_t i = 0; i < size (); i++)
	ostr << i << ": " << GetOrientedEdge (i) << endl;
    ostr << " Face attributes: ";
    face->PrintAttributes (ostr);
    return ostr.str ();
}

void OrientedFace::GetVertexSet (VertexSet* vertexSet) const
{
    GetFace ()->GetVertexSet (vertexSet);
}

void OrientedFace::GetEdgeSet (EdgeSet* edgeSet) const
{
    GetFace ()->GetEdgeSet (edgeSet);
}

bool OrientedFace::IsStandalone () const
{
    return GetFace ()->IsStandalone ();
}
