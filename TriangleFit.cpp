/**
 * @file   TriangleFit.h
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Declaration of the TriangleFit class
 */

#include "TriangleFit.h"
#include "Body.h"
#include "Data.h"
#include "Debug.h"
#include "DebugStream.h"


ostream& operator<< (ostream& ostr, const TriangleFit& fit)
{
    ostr << "TriangleFit: " << ((fit.m_margin == TriangleFit::BEFORE_FIRST) ? 
				"BEFORE_FIRST" : "AFTER_FIRST") << endl;
    ostr << fit.m_first << " - " << fit.m_second << endl;
    return ostr;
}

TriangleFit::Margin TriangleFit::FlipMargin (
    TriangleFit::Margin margin)
{
    return (margin == BEFORE_FIRST) ? AFTER_FIRST : BEFORE_FIRST;
}


void TriangleFit::GetTriangle (
    const FaceEdgeIndex& first, const FaceEdgeIndex& second,
    G3D::Vector3* triangle)
{
    size_t afterIndex = second.m_face->GetNextValidIndex (second.m_edgeIndex);
    triangle[0] = *second.m_face->GetEnd (afterIndex);
    triangle[1] = *second.m_face->GetBegin (afterIndex);
    size_t beforeIndex = first.m_face->GetPreviousValidIndex (
	first.m_edgeIndex);
    G3D::Vector3 otherTwo = *first.m_face->GetEnd (beforeIndex);
    RuntimeAssert (triangle[1] == otherTwo,
		   "Invalid triangle: ", triangle[1], ", ", otherTwo);
    triangle[2] = *first.m_face->GetBegin (beforeIndex);
}


void TriangleFit::Initialize (
    list<TriangleFit>* queue, Body* body)
{
    FaceEdgeIndex first, second;
    FaceEdgeIndex::TwoConnectedFaces (body, &first, &second);
    queue->push_back (
	TriangleFit (
	    TriangleFit::BEFORE_FIRST, first, second));
    queue->push_back (
	TriangleFit (
	    TriangleFit::AFTER_FIRST, first, second));
}

void TriangleFit::AddQueue (
    list<TriangleFit>* queue, TriangleFit*, OrientedFace* fit)
{
    FaceEdgeIndex first, second;
    FaceEdgeIndex::GetFaceIntersection (
	*fit, GetFirstFace (), &first, &second);
    queue->push_back (
	TriangleFit (TriangleFit::FlipMargin (
				    GetMargin ()), first, second));
    FaceEdgeIndex::GetFaceIntersection (
	*fit, GetSecondFace (), &first, &second);
    queue->push_back (TriangleFit (
			   GetMargin (), first, second));
}

void TriangleFit::GetPoints (G3D::Vector3* points) const
{
    if (GetMargin () == BEFORE_FIRST)
	GetTriangle (GetFirst (), GetSecond (), points);
    else
	GetTriangle (GetSecond (), GetFirst (), points);
}

OrientedFace* TriangleFit::FitAndDuplicateFace (Body* body) const
{
    using G3D::Vector3;
    Vector3 points[3];
    GetPoints (points);
    Vector3 translation;
    bool found = false;
    OrientedFace* of = 0;
    BOOST_FOREACH (of, body->GetOrientedFaces ())
	if (Body::FitFace (*of, points, 3, &translation))
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

TriangleFit TriangleFit::PopQueue (list<TriangleFit>* queue)
{
    TriangleFit fit = queue->front ();
    queue->pop_front ();
    return fit;
}
