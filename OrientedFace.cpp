/**
 * @file OrientedFace.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the OrientedFace class
 */

#include "AttributeInfo.h"
#include "Debug.h"
#include "Face.h"
#include "OrientedFace.h"
#include "OrientedEdge.h"
#include "Utils.h"
#include "Vertex.h"


OrientedFace::OrientedFace(const boost::shared_ptr<Face>& face, bool reversed) : 
    OrientedElement (face, reversed) 
{
}

boost::shared_ptr<Face> OrientedFace::GetFace () const
{
    return boost::static_pointer_cast<Face>(GetElement());
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

G3D::Plane OrientedFace::GetPlane () const
{
    G3D::Plane plane = GetFace ()->GetPlane ();
    if (IsReversed ())
	plane.flip ();
    return plane;
}

void OrientedFace::AddAdjacentBody (const boost::shared_ptr<Body>& body, 
				  size_t ofIndex) 
{
    GetFace ()->AddAdjacentBody (body, ofIndex);
}

const AdjacentBody& OrientedFace::GetAdjacentBody (bool faceReversed) const
{
    return GetFace ()->GetAdjacentBody (
	IsReversed () ^ faceReversed);
}

size_t OrientedFace::GetAdjacentBodySize () const
{
    return GetFace ()->GetAdjacentBodySize ();
}

void OrientedFace::UpdateAdjacentFace (const boost::shared_ptr<OrientedFace>& of)
{
    for (size_t i = 0; i < size (); i++)
    {
	const OrientedEdge& oe = GetOrientedEdge (i);
	oe.AddAdjacentOrientedFace (of, i);
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

float OrientedFace::GetArea () const
{
    return GetFace ()->GetArea ();
}

void OrientedFace::CalculatePerimeter ()
{
    GetFace ()->CalculatePerimeter ();
}

boost::shared_ptr<Vertex> OrientedFace::getBeginVertex (size_t edgeIndex) const
{
    const OrientedEdge& oe = GetOrientedEdge (edgeIndex);
    return oe.GetBeginPtr ();
}

boost::shared_ptr<Vertex> OrientedFace::getEndVertex (size_t edgeIndex) const
{
    const OrientedEdge& oe = GetOrientedEdge (edgeIndex);
    return oe.GetEndPtr ();
}

string OrientedFace::ToString (const AttributesInfo* ai) const
{
    ostringstream ostr;
    boost::shared_ptr<Face> face = GetFace ();
    ostr << "Oriented Face " << GetStringId () << " " 
	 << face->GetDuplicateStatus () << " useCount=" << face.use_count ()
	 << ": " << endl;
    ostr << size () << " edges part of the face:" << endl;
    for (size_t i = 0; i < size (); i++)
	ostr << i << ": " << GetOrientedEdge (i).ToStringShort () << endl;
    ostr << "Face attributes: ";
    if (face->HasAttributes ())
	face->PrintAttributes (ostr, ai);
    ostr << " center=" << GetFace ()->GetCenter () 
	 << " "
	 << endl;

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

QColor OrientedFace::GetColor (const QColor& defaultColor) const
{
    return GetFace ()->GetColor (defaultColor);
}

bool OrientedFace::HasConstraints () const
{
    return GetFace ()->HasConstraints ();
}

size_t OrientedFace::GetConstraintIndex (size_t i) const
{
    return GetFace ()->GetConstraintIndex (i);
}


void OrientedFace::GetVertexSetV (
    const vector<boost::shared_ptr<OrientedFace> >& vof,
    VertexSet* vertexSet)
{
    for_each (vof.begin (), vof.end (),
	      boost::bind (&OrientedFace::GetVertexSet, _1, vertexSet));
}

vtkSmartPointer<vtkPolyData> OrientedFace::GetPolyData (
    const vector<boost::shared_ptr<OrientedFace> >& vof)
{
    // create and set the points
    vtkSmartPointer<vtkPoints> polyPoints;
    vector<boost::shared_ptr<Vertex> > sortedPoints;
    getPolyPoints (vof, &polyPoints, &sortedPoints);
    VTK_CREATE (vtkPolyData, polyData);
    polyData->SetPoints (polyPoints);
    
    // create the cells
    size_t numberOfCells = vof.size ();
    polyData->Allocate (numberOfCells, numberOfCells);
    createPolyCells (vof, polyData, sortedPoints);
    return polyData;
}


void OrientedFace::getPolyPoints (
    const vector<boost::shared_ptr<OrientedFace> >& vof,
    vtkSmartPointer<vtkPoints>* polyPoints,
    vector<boost::shared_ptr<Vertex> >* sortedPoints)
{
    VertexSet vertexSet;
    OrientedFace::GetVertexSetV (vof, &vertexSet);
    sortedPoints->resize(vertexSet.size ());
    copy (vertexSet.begin (), vertexSet.end (), sortedPoints->begin ());


    *polyPoints = vtkPoints::New ();
    (*polyPoints)->SetNumberOfPoints (sortedPoints->size ());
    for (size_t i = 0; i < sortedPoints->size (); ++i)
    {
	G3D::Vector3 p = (*sortedPoints)[i]->GetVector ();
	(*polyPoints)->InsertPoint (i, p.x, p.y, p.z);
    }
}

void OrientedFace::createPolyCells (
    const vector<boost::shared_ptr<OrientedFace> >& vof,
    vtkSmartPointer<vtkPolyData> polyData, 
    const vector<boost::shared_ptr<Vertex> >& sortedPoints)
{
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, vof)
    {
	VTK_CREATE(vtkTriangle, aTriangle);
	for (size_t i = 0; i < 3; i++)
	{
	    boost::shared_ptr<Vertex> point = of->GetBeginVertex (i);
	    size_t pi = FindVertex (sortedPoints, point);
	    aTriangle->GetPointIds ()->SetId (i, pi);
	}
	polyData->InsertNextCell(aTriangle->GetCellType(), 
				 aTriangle->GetPointIds());
    }
}

