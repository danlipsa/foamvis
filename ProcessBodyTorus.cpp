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
    push (of);
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
    OrientedFace& of = *ofi.GetOrientedFace ();
    cdbg << "Edge part of " << oe.GetFacePartOfSize () 
	 << " faces" << endl
	 << oe << endl
	 << of << endl
	 << "Edge index: " << ofi.GetOrientedEdgeIndex () << endl;



    G3D::Vector3 translation;
    of.CalculateTranslation (oe, ofi.GetOrientedEdgeIndex (), &translation);
    if (! translation.isZero ())
    {
	Face* face = of.GetFace ();
	const G3D::Vector3& begin = *face->GetOrientedEdge (0)->GetBegin ();
	Face* f = m_foam->GetFaceDuplicate (*face, begin);
	const BodyIndex& bi = of.GetBodyPartOf ();
	OrientedFace* bodyOf = 
	    bi.GetBody ()->GetOrientedFace (bi.GetOrientedFaceIndex ());
	bodyOf->SetFace (f);

	cdbg << "Translation: Face index: " << bi.GetOrientedFaceIndex () << endl;

    }
    return true;
}


// Private Methods
// ======================================================================
void ProcessBodyTorus::push (OrientedFace* of)
{
    for (size_t i = 0; i < of->size (); i++)
	m_queue.push (OrientedFaceIndex (of, i));
}

bool ProcessBodyTorus::pop (
    OrientedEdge* orientedEdge, OrientedFaceIndex* orientedFaceIndex)
{
    OrientedFaceIndex srcOfi;
    while (m_queue.size () > 0)
    {
	srcOfi = m_queue.front ();
	m_queue.pop ();
	OrientedEdge destOe;
	srcOfi.GetOrientedEdge (&destOe);
	size_t srcBodyId = srcOfi.
	destOe.Reverse ();
	size_t i;
	for (i = 0; i < destOe.GetFacePartOfSize (); i++)
	{
	    const OrientedFaceIndex& destOfi = destOe.GetFacePartOf (i);
	    if (srcBodyId != destOfi.GetBodyId () ||
		destOe.IsReversed () != destOfi.IsOrientedEdgeReversed ())
		continue;

	    const OrientedFace& of = *destOfi.GetOrientedFace ();
	    const BodyIndex& bi = of.GetBodyPartOf ();
	    
	    if (! m_traversed[bi.GetOrientedFaceIndex ()])
	    {
		push (of);
		m_traversed[bi.GetOrientedFaceIndex ()] = true;
		*orientedEdge = destOe;
		*orientedFaceIndex = destOfi;
		return true;
	    }
	}
    }
    return false;
}
