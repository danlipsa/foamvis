/**
 * @file   FaceEdgeIndex.cpp
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Implementation of the FaceEdgeIndex class
 */

#include "FaceEdgeIndex.h"
#include "Face.h"
#include "OrientedFace.h"
#include "Body.h"
#include "Debug.h"
#include "Data.h"

ostream& operator<< (ostream& ostr, const FaceEdgeIndex& fei)
{
    Face* face = fei.m_face->GetFace ();
    ostr << "FaceEdgeIndex: face " << face->GetOriginalIndex () 
	 << " " << face->GetColor () << ", index "
	 << fei.m_edgeIndex;
    return ostr;
}

bool FaceEdgeIndex::GetFaceIntersection (
    const OrientedFace& firstFace, const OrientedFace& secondFace,
    FaceEdgeIndex* first, FaceEdgeIndex* second)
{
    for (size_t i = 0; i < firstFace.GetEdgeCount (); i++)
    {
	for (size_t j = 0; j < secondFace.GetEdgeCount (); j++)
	{
	    if (*(firstFace.GetOrientedEdge (i).GetEdge ()) == 
		*(secondFace.GetOrientedEdge (j).GetEdge ()))
	    {
		*first = FaceEdgeIndex (&firstFace, i);
		*second = FaceEdgeIndex (&secondFace, j);
		return true;
	    }
	}
    }
    /*
    cdbg << "First face:" << endl << firstFace << endl
	 << "Second face:" << endl << secondFace << endl;
    */
    return false;
}

void FaceEdgeIndex::Initialize (list<FaceEdgeIndex>* queue, Body* body)
{
    OrientedFace* of = body->GetOrientedFace (0);
    body->SetPlacedOrientedFace (static_cast<size_t>(0));
    size_t size = body->GetOrientedFaces().size ();
    for (size_t i = 0; i < size; i++)
	queue->push_back (FaceEdgeIndex (of, i));
}

void FaceEdgeIndex::TwoConnectedFaces (
    Body* body, FaceEdgeIndex* first, FaceEdgeIndex* second)
{
    bool found = false;
    for (size_t j = 1; j < body->GetOrientedFaces().size (); j++)
    {
	if (GetFaceIntersection (
		*body->GetOrientedFace (0), *body->GetOrientedFace (j),
		first, second))
	{
	    body->SetPlacedOrientedFace (static_cast<size_t>(0));
	    body->SetPlacedOrientedFace (j);
	    found = true;
	    break;
	}
    }
    RuntimeAssert (
	found, "The first face is not directly connected to any other face");
}

void FaceEdgeIndex::AddQueue (
    list<FaceEdgeIndex>* queue, OrientedFace* fit)
{
    size_t size = fit->GetFace ()->GetOrientedEdges ().size ();
    for (size_t i = 0; i < size; i++)
	if (i != m_edgeIndex)
	    queue->push_back (FaceEdgeIndex (fit, i));
}


OrientedFace* FaceEdgeIndex::FitAndDuplicateFace (Body* body) const
{
    using G3D::Vector3;
    Vector3 translation;
    bool found = false;
    OrientedFace* of = 0;
    BOOST_FOREACH (of, body->GetOrientedFaces ())
	if (Body::FitFace (*of, *this,  &translation))
	{
	    // you  only need  to consider  one match  because  of the
	    // orientation of the face.
	    found = true;
	    break;
	}
    RuntimeAssert (found, "No face was fitted for : ", *this);
    if (! translation.isZero ())
    {
	//found a possible fit
	of->SetFace (
	    body->GetData ()->GetFaceDuplicate (
		*of->GetFace (),
		*(of->GetFace ()->GetOrientedEdge(0)->GetBegin ()) + 
		translation));
    }
    return of;
}
