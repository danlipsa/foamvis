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
#include "OrientedEdge.h"
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
    OrientedFaceIndex ofi, nextOfi;
    if (! pop (&ofi, &nextOfi))
	return false;
    const OrientedEdge& oe = ofi.GetOrientedEdge ();
    const OrientedEdge& nextOe = nextOfi.GetOrientedEdge ();

    G3D::Vector3 translation;
    nextOe.CalculateTranslation (oe, &translation);
    if (! translation.isZero ())
    {
	Face* translatedNextFace = 
	    m_foam->GetFaceDuplicate (*nextOfi.GetFace (), translation);
	OrientedFace* nextOf = nextOfi.GetOrientedFace ();
	nextOf->SetFace (translatedNextFace);
    }
    else
    {
	OrientedFace* nextOf = nextOfi.GetOrientedFace ();
	cdbg << "Face " << nextOf->GetSignedIdString ()
	     << " does not need translation" << endl;
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
    OrientedFaceIndex* orientedFaceIndex,
    OrientedFaceIndex* nextOrientedFaceIndex)
{
    while (m_queue.size () > 0)
    {
	OrientedFaceIndex ofi = m_queue.front ();
	m_queue.pop ();
	OrientedEdge nextOe = ofi.GetOrientedEdge ();
	size_t bodyId = ofi.GetBodyId ();
	nextOe.Reverse ();
	size_t i;
	for (i = 0; i < nextOe.GetFacePartOfSize (); i++)
	{
	    const OrientedFaceIndex& nextOfi = nextOe.GetFacePartOf (i);
	    cdbg << "ofi: " << ofi << " nextOfi: " << nextOfi << endl;
		

	    OrientedFace* nextOf = nextOfi.GetOrientedFace ();
	    if (bodyId != nextOfi.GetBodyId ())
	    {
		cdbg << "wrong body" << endl;
		continue;
	    }
	    if (nextOe.IsReversed () != nextOfi.IsOrientedEdgeReversed ())
	    {
		cdbg << "wrong orientation" << endl;
		continue;
	    }
	    if (! ofi.IsValidNext (nextOfi))
	    {
		cdbg << "wrong angle around edge" << endl;
		continue;
	    }
	    const BodyIndex& nextBi = nextOf->GetBodyPartOf ();
	    if (m_traversed[nextBi.GetOrientedFaceIndex ()])
	    {
		cdbg << "already traversed" << endl;
		continue;
	    }
	    push (nextOf);
	    m_traversed[nextBi.GetOrientedFaceIndex ()] = true;
	    *orientedFaceIndex = ofi;
	    *nextOrientedFaceIndex = nextOfi;
	    return true;
	}
    }
    return false;
}
