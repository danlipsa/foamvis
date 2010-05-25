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

void ProcessBodyTorus::Unwrap ()
{
    Initialize ();
    while (Step ())
	;
}

bool ProcessBodyTorus::Step ()
{
    using G3D::Vector3int16;
    OrientedFaceIndex ofi, nextOfi;
    if (! pop (&ofi, &nextOfi))
	return false;
    const OrientedEdge& oe = ofi.GetOrientedEdge ();
    const OrientedEdge& nextOe = nextOfi.GetOrientedEdge ();
    const OOBox& periods = m_foam->GetPeriods ();

    G3D::Vector3int16 translation = 
	periods.GetTranslation (*nextOe.GetBegin (), *oe.GetEnd ());
    if (translation != Vector3int16 (0, 0, 0))
    {
	Face* translatedNextFace = 
	    m_foam->GetFaceDuplicate (*nextOfi.GetFace (), translation);
	OrientedFace* nextOf = nextOfi.GetOrientedFace ();
	nextOf->SetFace (translatedNextFace);
	//cdbg << "    Face " << nextOf->GetStringId ()
	//     << " translated " << translation << endl;
    }
    else
    {
	//cdbg << "    Face " << nextOfi.GetOrientedFace ()->GetStringId ()
	//     << " does not need translation" << endl;
    }
    //cdbg << endl;
    return true;
}


// Private Methods
// ======================================================================
void ProcessBodyTorus::push (OrientedFace* of)
{
    for (size_t i = 0; i < of->size (); i++)
	m_queue.push (OrientedFaceIndex (of, i));
}

void ProcessBodyTorus::restrictFacesAroundAnEdge (
    const OrientedFaceIndex& ofi, 
    vector<OrientedFaceIndex>* possibilities)
{
    OrientedEdge nextOe = ofi.GetOrientedEdge ();
    size_t bodyId = ofi.GetBodyId ();
    nextOe.Reverse ();
    size_t i;
/*
    cdbg << " ---------- Trying " << nextOe.GetFacePartOfSize ()
	 << " possibilities ----------    " << "ofi: " << ofi << endl;
*/
    for (i = 0; i < nextOe.GetFacePartOfSize (); i++)
    {
	const OrientedFaceIndex& nextOfi = nextOe.GetFacePartOf (i);
	//cdbg << " nextOfi: " << nextOfi << endl;

	if (bodyId != nextOfi.GetBodyId ())
	{
	    //cdbg << "wrong body" << endl;
	    continue;
	}
	if (nextOe.IsReversed () != nextOfi.IsOrientedEdgeReversed ())
	{
	    //cdbg << "wrong orientation" << endl;
	    continue;
	}
	//cdbg << "stored for later" << endl;
	possibilities->push_back (nextOfi);
    }
}


bool ProcessBodyTorus::chooseFaceNeighbor (
    const OrientedFaceIndex& ofi, 
    const vector<OrientedFaceIndex>& possibilities,
    OrientedFaceIndex* nextOrientedFaceIndex)
{
    OrientedFaceIndex nextOfi;
    RuntimeAssert (possibilities.size () <= 2,
		   "More possibilities than we can discern");
    if (possibilities.size () == 0)
	return false;
    BOOST_FOREACH (nextOfi, possibilities)
    {
	OrientedFace* nextOf = nextOfi.GetOrientedFace ();
	const BodyIndex& nextBi = nextOf->GetBodyPartOf ();
	if (m_traversed[nextBi.GetOrientedFaceIndex ()])
	{
	    //cdbg << "already traversed" << endl;
	    continue;
	}

	if (possibilities.size () > 1 &&
	    ! ofi.IsValidNext (nextOfi))
	{
	    //cdbg << "wrong angle around edge" << endl;
	    continue;
	}
	*nextOrientedFaceIndex = nextOfi;
	return true;
    }
    return false;
}


bool ProcessBodyTorus::pop (
    OrientedFaceIndex* orientedFaceIndex,
    OrientedFaceIndex* nextOrientedFaceIndex)
{

    while (m_queue.size () > 0)
    {
	vector<OrientedFaceIndex> possibilities;
	OrientedFaceIndex ofi = m_queue.front ();
	m_queue.pop ();
	restrictFacesAroundAnEdge (ofi, &possibilities);
	if (chooseFaceNeighbor (ofi, possibilities, nextOrientedFaceIndex))
	{

	    OrientedFace* nextOf = nextOrientedFaceIndex->GetOrientedFace ();
	    const BodyIndex& nextBi = nextOf->GetBodyPartOf ();
	    push (nextOf);
	    m_traversed[nextBi.GetOrientedFaceIndex ()] = true;
	    *orientedFaceIndex = ofi;
	    return true;
	}
    }
    return false;
}
