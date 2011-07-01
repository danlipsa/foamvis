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
#include "Utils.h"
#include "Vertex.h"

ProcessBodyTorus::ProcessBodyTorus (const Foam& foam, 
				    const boost::shared_ptr<Body>& body) : 
    m_foam (foam), m_body (body), m_traversed (body->size (), false)
{}

void ProcessBodyTorus::Initialize ()
{
    boost::shared_ptr<OrientedFace> of = m_body->GetOrientedFacePtr (0);
    m_traversed[0] = true;
    push (of);
}

void ProcessBodyTorus::Unwrap (
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    Initialize ();
    while (Step (vertexSet, edgeSet, faceSet))
	;
}

bool ProcessBodyTorus::Step (
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    using G3D::Vector3int16;
    OrientedFaceIndex ofi, nextOfi;
    if (! pop (&ofi, &nextOfi))
	return false;
    const OrientedEdge& oe = ofi.GetOrientedEdge ();
    const OrientedEdge& nextOe = nextOfi.GetOrientedEdge ();
    const OOBox& periods = m_foam.GetOriginalDomain ();

    G3D::Vector3int16 translation = 
	periods.GetTranslation (nextOe.GetBeginVector (), 
				oe.GetEndVector ());
    if (translation != Vector3int16Zero)
    {
	boost::shared_ptr<Face>  translatedNextFace = 
	    nextOfi.GetFace ()->GetDuplicate (
		m_foam.GetOriginalDomain (), translation,
		vertexSet, edgeSet, faceSet);
	boost::shared_ptr<OrientedFace>  nextOf = nextOfi.GetOrientedFace ();
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

void ProcessBodyTorus::push (boost::shared_ptr<OrientedFace> of)
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
	vector<OrientedFaceIndex> possibilities;
	OrientedFaceIndex ofi = m_queue.front ();
	m_queue.pop ();
	restrictFacesAroundAnEdge (ofi, &possibilities);
	if (chooseFaceNeighbor (ofi, possibilities, nextOrientedFaceIndex))
	{

	    boost::shared_ptr<OrientedFace> nextOf = 
		nextOrientedFaceIndex->GetOrientedFace ();
	    const BodyIndex& nextBi = nextOf->GetBodyPartOf ();
	    push (nextOf);
	    m_traversed[nextBi.GetOrientedFaceIndex ()] = true;
	    *orientedFaceIndex = ofi;
	    return true;
	}
    }
    return false;
}


void ProcessBodyTorus::restrictFacesAroundAnEdge (
    const OrientedFaceIndex& ofi, 
    vector<OrientedFaceIndex>* possibilities)
{
    OrientedEdge oe = ofi.GetOrientedEdge ();
    size_t bodyId = ofi.GetBodyId ();
    oe.Reverse ();
/*
    cdbg << " ---------- Trying " << oe.GetFacePartOfSize ()
	 << " possibilities ----------    " << "ofi: " << ofi << endl;
*/


    for (OrientedFaceIndexList::const_iterator 
	     it = oe.GetFacePartOfBegin ();
	 it != oe.GetFacePartOfEnd (); it++)
    {
	const OrientedFaceIndex& nextOfi = *it;
	if (nextOfi.IsStandalone ())
	    continue;
	//cdbg << " nextOfi: " << nextOfi << endl;

	if (bodyId != nextOfi.GetBodyId ())
	{
	    //cdbg << "wrong body" << endl;
	    continue;
	}
	if (oe.IsReversed () != nextOfi.IsOrientedEdgeReversed ())
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
    RuntimeAssert (
	possibilities.size () <= 2,
	"ProcessBodyTorus: more possibilities than we can discern: ", 
	possibilities.size (), " (should be <= 2)");
    if (possibilities.size () == 0)
	return false;
    BOOST_FOREACH (nextOfi, possibilities)
    {
	boost::shared_ptr<OrientedFace>  nextOf = nextOfi.GetOrientedFace ();
	const BodyIndex& nextBi = nextOf->GetBodyPartOf ();
	if (m_traversed[nextBi.GetOrientedFaceIndex ()])
	{
	    //cdbg << "already traversed" << endl;
	    continue;
	}

	if (possibilities.size () > 1 && ! ofi.IsValidNext (nextOfi))
	{
	    //cdbg << "wrong angle around edge" << endl;
	    continue;
	}
	*nextOrientedFaceIndex = nextOfi;
	return true;
    }
    return false;
}
