/**
 * @file   ProcessBodyTorus.cpp
 * @author Dan R. Lipsa
 * @date 22 March 2010
 *
 * Implementation of the ProcessBodyTorus class
 */

#include "Body.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Foam.h"
#include "Face.h"
#include "ProcessBodyTorus.h"
#include "OrientedFace.h"
#include "Vertex.h"

ProcessBodyTorus::ProcessBodyTorus (Foam* foam, Body* body) : 
    m_foam (foam), m_body (body), m_traversed (body->size (), false)
{}

void ProcessBodyTorus::Initialize ()
{
    OrientedFace* of = m_body->GetOrientedFace (0);
    m_traversed[0] = true;
    push (*of);
}

void ProcessBodyTorus::Cleanup ()
{
    m_body->ClearPartOf ();
}


bool ProcessBodyTorus::Step ()
{
    OrientedEdge oe;
    OrientedFaceIndex ofi;
    if (! pop (&oe, &ofi))
	return false;
    OrientedFace of (ofi.m_face, ofi.m_faceReversed);

    cdbg << "Edge part of " << oe.GetFacePartOfSize () 
	 << " faces" << endl
	 << oe << endl
	 << of << endl
	 << "Edge index: " << ofi.m_orientedEdgeIndex << endl;



    G3D::Vector3 translation;
    of.CalculateTranslation (oe, ofi.m_orientedEdgeIndex, &translation);
    if (! translation.isZero ())
    {
	Face* face = of.GetFace ();
	const G3D::Vector3& begin = *face->GetOrientedEdge (0)->GetBegin ();
	Face* f = m_foam->GetFaceDuplicate (*face, begin);
	const BodyIndex& bi = of.GetBodyPartOf ();
	OrientedFace* bodyOf = 
	    bi.m_body->GetOrientedFace (bi.m_orientedFaceIndex);
	bodyOf->SetFace (f);

	cdbg << "Translation: Face index: " << bi.m_orientedFaceIndex << endl;

    }
    return true;
}


// Private Methods
// ======================================================================
void ProcessBodyTorus::push (const OrientedFace& of)
{
    for (size_t i = 0; i < of.size (); i++)
    {
	OrientedEdge oe;
	of.GetOrientedEdge (i, &oe);
	oe.Reverse ();
	m_queue.push (oe);
    }
}

bool ProcessBodyTorus::pop (
    OrientedEdge* orientedEdge, OrientedFaceIndex* orientedFaceIndex)
{
    OrientedEdge oe;
    while (m_queue.size () > 0)
    {
	oe = m_queue.front ();
	m_queue.pop ();
	size_t i;
	for (i = 0; i < oe.GetFacePartOfSize (); i++)
	{
	    const OrientedFaceIndex& ofi = oe.GetFacePartOf (i);
	    OrientedFace of (ofi.m_face, ofi.m_faceReversed);
	    const BodyIndex& bi = of.GetBodyPartOf ();
	    
	    if (! m_traversed[bi.m_orientedFaceIndex])
	    {
		push (of);
		m_traversed[bi.m_orientedFaceIndex] = true;
		*orientedEdge = oe;
		*orientedFaceIndex = ofi;
		return true;
	    }
	}
    }
    return false;
}
