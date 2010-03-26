/**
 * @file   FaceEdgeIndex.cpp
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Implementation of the FaceEdgeIndex class
 */

#include "Body.h"
#include "Debug.h"
#include "Data.h"
#include "Edge.h"
#include "FaceEdgeIndex.h"
#include "Face.h"
#include "OrientedFace.h"

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
    for (size_t i = 0; i < firstFace.size (); i++)
    {
	for (size_t j = 0; j < secondFace.size (); j++)
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
	    found = true;
	    break;
	}
    }
    RuntimeAssert (
	found, "The first face is not directly connected to any other face");
}

