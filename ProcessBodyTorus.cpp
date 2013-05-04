/**
 * @file   ProcessBodyTorus.cpp
 * @author Dan R. Lipsa
 * @date 22 March 2010
 *
 * Implementation of the ProcessBodyTorus class
 */

#include "Body.h"
#include "Debug.h"
#include "Foam.h"
#include "Face.h"
#include "ProcessBodyTorus.h"
#include "OrientedFace.h"
#include "OrientedEdge.h"
#include "OOBox.h"
#include "Utils.h"
#include "Vertex.h"

ProcessBodyTorus::ProcessBodyTorus (const Foam& foam, 
				    const boost::shared_ptr<Body>& body) : 
    m_foam (foam), m_body (body), m_traversed (body->GetFaceCount (), false)
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
    AdjacentOrientedFace aof, nextAof;
    if (! pop (&aof, &nextAof))
	return false;
    const OrientedEdge& oe = aof.GetOrientedEdge ();
    const OrientedEdge& nextOe = nextAof.GetOrientedEdge ();
    const OOBox& periods = m_foam.GetTorusDomain ();

    G3D::Vector3int16 translation = 
	periods.GetTranslation (nextOe.GetBeginVector (), oe.GetEndVector ());
    if (translation != Vector3int16Zero)
    {
	boost::shared_ptr<Face>  translatedNextFace = 
	    nextAof.GetFace ()->GetDuplicate (
		m_foam.GetTorusDomain (), translation,
		vertexSet, edgeSet, faceSet);
	boost::shared_ptr<OrientedFace>  nextOf = nextAof.GetOrientedFace ();
	nextOf->SetFace (translatedNextFace);
	__LOG__(
	    cdbg << "    Face " << nextOf->GetStringId ()
	    << " translated " << translation << endl;);
    }
    else
    {
	__LOG__(
	    cdbg << "    Face " << nextAof.GetOrientedFace ()->GetStringId ()
	    << " does not need translation" << endl;);
    }
    __LOG__(
	cdbg << endl;);
    return true;
}

void ProcessBodyTorus::push (boost::shared_ptr<OrientedFace> of)
{
    for (size_t i = 0; i < of->size (); i++)
	m_queue.push (AdjacentOrientedFace (of, i));
}

bool ProcessBodyTorus::pop (AdjacentOrientedFace* adjacentOrientedFace,
			    AdjacentOrientedFace* nextAdjacentOrientedFace)
{

    while (! m_queue.empty ())
    {
	vector<AdjacentOrientedFace> possibilities;
	AdjacentOrientedFace aof = m_queue.front ();
	m_queue.pop ();
	restrictFacesAroundAnEdge (aof, &possibilities);
	if (chooseFaceNeighbor (aof, possibilities, nextAdjacentOrientedFace))
	{

	    boost::shared_ptr<OrientedFace> nextOf = 
		nextAdjacentOrientedFace->GetOrientedFace ();
	    const AdjacentBody& nextAb = nextOf->GetAdjacentBody ();
	    push (nextOf);
	    m_traversed[nextAb.GetOrientedFaceIndex ()] = true;
	    *adjacentOrientedFace = aof;
	    return true;
	}
    }
    return false;
}


void ProcessBodyTorus::restrictFacesAroundAnEdge (
    const AdjacentOrientedFace& aof, 
    vector<AdjacentOrientedFace>* possibilities)
{
    OrientedEdge oe = aof.GetOrientedEdge ();
    size_t bodyId = aof.GetBodyId ();
    oe.Reverse ();
    const AdjacentOrientedFaces& aofs = oe.GetAdjacentOrientedFaces ();

    __LOG__ (
	cdbg << " ---------- Trying " << aofs.size ()
	<< " possibilities ----------    " << "aof: " << aof << endl;);

    for (AdjacentOrientedFaces::const_iterator it = aofs.begin ();
	 it != aofs.end (); ++it)
    {
	const AdjacentOrientedFace& nextAof = *it;
        __LOG__ (cdbg << "nextFace: " << nextAof << endl;);

	if (nextAof.IsStandalone ())
	{
	    __LOG__ (cdbg << "standalone face" << endl;);
	    continue;
	}
	
	if (bodyId != nextAof.GetBodyId ())
	{
	    __LOG__ (cdbg << "wrong body" << endl;);
	    continue;
	}
	if (oe.IsReversed () != nextAof.IsOrientedEdgeReversed ())
	{
	    __LOG__ (
		cdbg << "wrong orientation" << endl;);
	    continue;
	}
	//cdbg << "stored for later" << endl;
	possibilities->push_back (nextAof);
    }
}


bool ProcessBodyTorus::chooseFaceNeighbor (
    const AdjacentOrientedFace& aof, 
    const vector<AdjacentOrientedFace>& possibilities,
    AdjacentOrientedFace* nextAdjacentOrientedFace)
{
    AdjacentOrientedFace nextAof;
    RuntimeAssert (
	possibilities.size () <= 2,
	"ProcessBodyTorus: more possibilities than we can discern: ", 
	possibilities.size (), " (should be <= 2)");
    BOOST_FOREACH (nextAof, possibilities)
    {
	boost::shared_ptr<OrientedFace>  nextOf = nextAof.GetOrientedFace ();
	const AdjacentBody& nextAb = nextOf->GetAdjacentBody ();
	if (m_traversed[nextAb.GetOrientedFaceIndex ()])
	{
	    __LOG__(
		cdbg << "\talready traversed" << endl;);
	    continue;
	}

	if (possibilities.size () > 1 && ! aof.IsValidNext (nextAof))
	{
	    __LOG__(
		cdbg << "\twrong angle around edge: " << nextAof << endl;);
	    continue;
	}
	*nextAdjacentOrientedFace = nextAof;
	__LOG__(
	    cdbg << "face(" << possibilities.size () << " possibilities) " 
	    << aof << " next face: " << nextAof << endl;);
	return true;
    }
    __LOG__(
	cdbg << "face(" << possibilities.size () 
	<< " possibilities) discarded: " << endl;);
    return false;
}
