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
#include "ProcessBodyTorus.h"
#include "EdgeFit.h"
#include "OrientedFace.h"

void ProcessBodyTorus::Initialize ()
{
    const Body::NormalFaceMap& map (m_body->GetNormalFaceMap ());
    cdbg << "Map " << map.size () << " elements" << endl;
    pair<G3D::Vector3, OrientedFace*> p;
    BOOST_FOREACH (p, map)
    {
	cdbg << "normal: " << p.first << endl
	     << *p.second << endl;
    }

    const OrientedFace* of = m_body->GetFirstFace ();
    addQueue (of, of->end ());
}

bool ProcessBodyTorus::Step ()
{
    Body::NormalFaceMap::iterator normalFaceIt = 
	m_body->GetCurrentNormalFace ();
    if ( normalFaceIt == m_body->GetNormalFaceMap ().end ())
	return false;
	
    cdbg << "Queue " << m_queue.size () << " elements" << endl;
    ostream_iterator<EdgeFit> output (cdbg, "\n");

    copy (m_queue.begin (), m_queue.end (), output);

    // place the face that fits over that margin (this might
    // create a duplicate face)
    RuntimeAssert (m_queue.size () > 0,
		   "Process body torus queue is empty");
    OrientedFace::const_iterator fitPosition;
    OrientedFace* face = m_body->FitFromQueue (&m_queue, &fitPosition);
	
    if (face->GetFace ()->IsDuplicate ())
	cdbg << "Fitted face: " << endl << *face << endl
	     << "normal: " << face->GetNormal () << endl;
    else
    {
	Face& f = *face->GetFace ();
	cdbg << "Fitted face " << f.GetOriginalIndex () 
	     << " " << f.GetColor () << " "
	     << " not a DUPLICATE" << endl;
    }
    addQueue (face, fitPosition);
    return true;
}


void ProcessBodyTorus::addQueue (const OrientedFace* of,
				 OrientedFace::const_iterator fitPosition)
{
    using G3D::Vector3;
    Vector3 normal = of->GetNormal ();
    for (OrientedFace::const_iterator it = of->begin (); it != of->end (); ++it)
    {
	if (it == fitPosition)
	    continue;
	OrientedEdge oe = *it;
	G3D::Vector3 edgeVector = oe.GetEdgeVector ();
	m_queue.push_front (
	    EdgeFit(oe, edgeVector.cross (normal).unit ()));
    }
    m_body->IncrementNormalFace ();
}

